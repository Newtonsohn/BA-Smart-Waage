using Backend.Application.Bins.UpdateWeight;
using Linux.Bluetooth;
using Linux.Bluetooth.Extensions;
using System.Text;
using System.Text.Json;

namespace EdgeDevice.BLE
{
    public enum State
    {
        Uninitialized,
        ServicesResolved,

    }

    public class BleDevice(ILogger<BleDevice> _logger, IHttpClientFactory _httpClientFactory)
    {
        private const int WeightByteLength = 4;
        private static readonly string ServiceUUID = "f0cbd08a-41a1-4b14-b66f-420e6c7f6d1f";
        private static readonly string CONFIG_UUID = "b4520837-7d4f-4e4a-96ff-8f9cd9e64577";
        private static readonly string MEASURE_UUID = "a03cfc2e-370e-4c34-9d8e-9d75f6e93e88";
        private GattCharacteristic? _measurementCharacteristic = null;

        public void Setup(string macAddress, string alias)
        {
            MacAddress = macAddress;
            Alias = alias;
        }

        public string MacAddress { get; private set; } = string.Empty;
        public string Alias { get; private set; } = string.Empty;


        public void OnDeviceFound(Device device)
        {
            device.Disconnected += OnDisconnect;
            device.ServicesResolved += OnServiceResolved;
        }

        private async Task OnDisconnect(Device device, BlueZEventArgs e)
        {
            device.Disconnected -= OnDisconnect;
            device.ServicesResolved -= OnServiceResolved;

            _logger.LogWarning("Disconnecting BLE device from bin with {MacAddress}", MacAddress);

            if (_measurementCharacteristic is null)
            {
                _logger.LogWarning("_measurementCharacteristic is null");
                return;
            }

            _measurementCharacteristic.Value -= OnMeasureValueChanged;
            await _measurementCharacteristic.StopNotifyAsync();

        }
        
        private async Task OnServiceResolved(Device device, BlueZEventArgs e)
        {
            var targetService = await device.GetServiceAsync(ServiceUUID);
            if (targetService == null)
            {
                _logger.LogError($"Target service {ServiceUUID} not found.");
                return;
            }

            await CheckConfiguraiton(targetService);
            await RegisterMeasurement(targetService);
        }

        private async Task RegisterMeasurement(IGattService1 targetService)
        {
            GattCharacteristic? measureChar = await targetService.GetCharacteristicAsync(MEASURE_UUID);
            if (measureChar is not null)
            {
                measureChar.Value += OnMeasureValueChanged;
                await measureChar.StartNotifyAsync();
                _measurementCharacteristic = measureChar;
            }
            else
            {
                _logger.LogError("Measurement characteristic not found on device {MacAddress}", MacAddress);
            }
        }

        private async Task CheckConfiguraiton(IGattService1 targetService)
        {
            var configChar = await targetService.GetCharacteristicAsync(CONFIG_UUID);
            if (configChar is null)
            {
                _logger.LogError("Config characteristic not found.");
                return;
            }

            var content = await configChar.ReadValueAsync(new Dictionary<string, object>());
            if (content is null)
            {
                _logger.LogError("Content could not be read from characteristics: {CONFIG_UUID}", CONFIG_UUID);
                return;
            }

            bool sendConfiguration = content.Length == 1 && content[0] > 0;
            if (!sendConfiguration)
            {
                return;
            }

            using var client = _httpClientFactory.CreateClient("BackendClient");
            var response = await client.GetAsync($"/bins/configuration?macAddress={MacAddress}");
            if (!response.IsSuccessStatusCode)
            {
                _logger.LogError("Failed to fetch configuration from backend. StatusCode: {StatusCode}", response.StatusCode);
                return;
            }

            //var configuration = await response.Content.ReadFromJsonAsync<SmartBinConfigurationResponse>()
            //                    ?? throw new InvalidCastException("Response could not be casted to SmartBinConfigurationResponse");


            var jsonsstring = await response.Content.ReadAsStringAsync();
            _logger.LogWarning("Reveived configuration: {Configuration}", jsonsstring);

            var options = new Dictionary<string, object>
            {
                { "type", "request" }
            };

            await configChar.WriteValueAsync(Encoding.UTF8.GetBytes(jsonsstring), options);
        }
    
        private async Task OnMeasureValueChanged(GattCharacteristic c, GattCharacteristicValueEventArgs ev)
        {
            if (ev.Value.Length == WeightByteLength)
            {
                float weight = BitConverter.ToSingle(ev.Value, 0);
                _logger.LogInformation($"[{DateTime.Now}] Weight: {weight:F2} g");
                using var client = _httpClientFactory.CreateClient("BackendClient");
                var palyoad = new UpdateBinWeightCommand(weight, MacAddress);

                var jsonPayload = JsonSerializer.Serialize(palyoad);
                var content = new StringContent(jsonPayload, Encoding.UTF8, "application/json");
                var response = await client.PutAsync("/bins/currentweight", content);
                if(!response.IsSuccessStatusCode)
                {
                    _logger.LogError("Bin with MAC: {MacAddress} could not be updated. {HttpSatusCode}", MacAddress, response.StatusCode);
                }
                else
                {
                    _logger.LogInformation("Bin with MAC: {MacAddress} was updated.", MacAddress);
                }
            }
            else
            {
                _logger.LogError("Invalid weight payload for device {MacAddress}", MacAddress);
            }
        }
    }
}
