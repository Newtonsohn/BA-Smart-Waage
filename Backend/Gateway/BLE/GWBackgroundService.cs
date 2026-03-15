using Backend.Application.Bins.UpdateWeight;
using Backend.Domain.Kernel;
using EdgeDevice.Network;
using Linux.Bluetooth;
using Linux.Bluetooth.Extensions;
using System.Text;
using System.Text.Json;
namespace EdgeDevice.BLE
{
    public class GWBackgroundService(ILogger<GWBackgroundService> _logger, IHttpClientFactory _httpClientFactory, IServiceProvider _serviceProvider) : BackgroundService, IBLEService
    {
        private ISet<string> _assignedDevices = new HashSet<string>();
        private TaskCompletionSource<bool> _adapterIsPoweredOn = new TaskCompletionSource<bool>();
        private TaskCompletionSource<bool> _deviceIsDeisconected = new TaskCompletionSource<bool>();
        private int _consecutiveFailures = 0;
        private const int MaxConsecutiveFailures = 3;
        private const int CooldownAfterFailureMs = 10000;

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var adapter = (await BlueZManager.GetAdaptersAsync()).FirstOrDefault();
            if (adapter == null)
            {
                _logger.LogError("No Bluetooth adapter found.");
                return;
            }

            _logger.LogInformation($"Using Bluetooth adapter: {adapter.Name}");
            adapter.PoweredOn += AdapterPoweredOnAsync;
            await _adapterIsPoweredOn.Task;

            while(!stoppingToken.IsCancellationRequested)
            {
                await Task.Delay(1000, stoppingToken);
                foreach (string targetAddress in _assignedDevices)
                {
                    await EnsureDiscoveryIsActiveAsync(adapter);
                    
                    var devices = await adapter.GetDevicesAsync();
                    foreach (var device in devices)
                    {
                        try
                        {
                            if (device is null)
                                continue;

                            var address = await device.GetAddressAsync();
                            if (address == null)
                                continue;

                            if (targetAddress.Equals(address))
                            {
                                await HandleDevice(adapter, device, address);
                                break;
                            }
                        }
                        catch(Exception ex)
                        {
                            _logger.LogError(ex, "Unexcepeceted error occured");
                            _consecutiveFailures++;
                            if (_consecutiveFailures >= MaxConsecutiveFailures)
                            {
                                _logger.LogWarning("Too many consecutive BLE failures ({Count}), cooling down for {Ms}ms", _consecutiveFailures, CooldownAfterFailureMs);
                                await adapter.StopDiscoveryAsync().ConfigureAwait(false);
                                await Task.Delay(CooldownAfterFailureMs, stoppingToken);
                                _consecutiveFailures = 0;
                            }
                            continue;
                        }
                    }
                }
            }
            _logger.LogInformation("Background job is done.");
        }

        private async Task HandleDevice(Adapter adapter, Device dev, string addr)
        {
            _logger.LogInformation("Targetdevice found: {Device}", dev.ObjectPath.ToString());
            await adapter.StopDiscoveryAsync();

            // Check for advertisement mode (timer wake-up): manufacturer data contains weight
            var manufacturerData = await dev.GetManufacturerDataAsync();
            if (manufacturerData != null && manufacturerData.Count > 0)
            {
                var payload = manufacturerData.Values.First() as byte[];
                if (payload != null && payload.Length >= 6)
                {
                    float weight = BitConverter.ToSingle(payload, 2);
                    _logger.LogInformation("Advertisement weight received: {Weight:F2} g from {Address}", weight, addr);

                    using var client = _httpClientFactory.CreateClient("BackendClient");
                    var command = new UpdateBinWeightCommand(weight, addr);
                    var content = new StringContent(JsonSerializer.Serialize(command), Encoding.UTF8, "application/json");
                    var response = await client.PutAsync("/bins/currentweight", content);
                    if (!response.IsSuccessStatusCode)
                        _logger.LogError("Failed to update weight for {Address}: {StatusCode}", addr, response.StatusCode);
                    else
                        _logger.LogInformation("Weight updated for {Address}: {Weight:F2} g", addr, weight);

                    _consecutiveFailures = 0;
                }
                return; // Do not connect in advertisement mode
            }

            // GATT mode (power-cycle): connect for configuration
            _logger.LogInformation("found device {Address}, connecting for configuration...", addr);
            var bleDevice = _serviceProvider.GetService<BleDevice>();
            var alias = await dev.GetAliasAsync();

            bleDevice!.Setup(addr, alias);
            bleDevice.OnDeviceFound(dev);

            dev.Connected += DeviceConnectedAsync;
            dev.Disconnected += DeviceDisconnectedAsync;

            _logger.LogInformation($"Connecting to {addr}...");
            _deviceIsDeisconected = new TaskCompletionSource<bool>();
            await Task.Delay(1000);
            await dev.ConnectAsync();
            _consecutiveFailures = 0;
            _logger.LogInformation("Wait for disconnecting");
            await _deviceIsDeisconected.Task;
        }

        private async Task EnsureDiscoveryIsActiveAsync(Adapter adapter)
        {
            if (!await adapter.GetDiscoveringAsync())
            {
                await adapter.StartDiscoveryAsync();
                _logger.LogInformation("Starting Discovery...");
            }
            await Task.Delay(100);
        }

        public override async Task StartAsync(CancellationToken cancellationToken)
        {
            _logger.LogInformation("Service starting: fetching assigned devices...");

            try
            {
                using var client = _httpClientFactory.CreateClient("BackendClient");
                var networkInformations = _serviceProvider.GetRequiredService<INetworkInformationProvider>();

                var macaddress = networkInformations.GetMacAddress().Value;
                _logger.LogInformation("Fetch assinged bins for mac address: {MacAddress}", macaddress);
                var response = await client.GetAsync($"/gateways/assignedBins?mac={macaddress}", cancellationToken);
                if (response.IsSuccessStatusCode)
                {
                    var devices = await response.Content.ReadFromJsonAsync<ISet<string>>(cancellationToken);
                    if (devices != null)
                    {
                        _logger.LogInformation("Fetched {Count} assigned devices", devices.Count);
                        _assignedDevices = devices;
                                               
                        _logger.LogWarning("Following devices are assinged to this gateway: ");
                        foreach (var dev in devices)
                        {
                            _logger.LogWarning(dev);
                        }
                    }
                }
                else
                {
                    _logger.LogError("Failed to fetch assigned devices. Status code: {StatusCode}", response.StatusCode);
                }
            }
            catch(Exception ex)
            {
                _logger.LogError(ex, "Unexpected error during StartAsync: ");
            }

            await base.StartAsync(cancellationToken);
        }

        private Task AdapterPoweredOnAsync(Adapter adapter, BlueZEventArgs e)
        {
            try
            {
                _adapterIsPoweredOn.SetResult(true);
                _logger.LogInformation("Adapter powered on.");
            }
            catch (Exception ex)
            {
                _logger.LogError("Discovery failed: {Message}", ex.Message);
            }
            return Task.CompletedTask;
        }

        private async Task DeviceConnectedAsync(Device device, BlueZEventArgs e)
        {
            var addr = await device.GetAddressAsync();
            _logger.LogInformation($"Connected to {addr}");
        }

        private async Task DeviceDisconnectedAsync(Device device, BlueZEventArgs e)
        {
            string? addr = null;
            addr = await device.GetAddressAsync();

            _logger.LogInformation($"Disconnected from {addr}");

            device.Connected -= DeviceConnectedAsync;
            device.Disconnected -= DeviceDisconnectedAsync;

            try
            {
                var adapter = (await BlueZManager.GetAdaptersAsync()).FirstOrDefault();
                if (adapter != null)
                {
                    await adapter.RemoveDeviceAsync(device.ObjectPath);
                    _logger.LogInformation($"Removed {addr} from adapter's cache.");
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to remove device with address: {addr}", addr);
            }
            finally
            {
                _deviceIsDeisconected.SetResult(true);
            }

            _logger.LogInformation($"Removed {addr} from dictionary.");
        }


        public Task<Result> UpdatedAssignedDevicesAsync(ISet<string> macAddresses)
        {
            _logger.LogInformation("new bins assinged:");
            foreach (var addr in macAddresses)
            {
                _logger.LogInformation(addr);
            }
            _assignedDevices = macAddresses;
            return Task.FromResult(Result.Success());
        }

        public Task<Result> RegisterSmartBinsAsync(ISet<string> macAddresses)
        {
            _assignedDevices = macAddresses;
            return Task.FromResult(Result.Success());
        }
    }
}

