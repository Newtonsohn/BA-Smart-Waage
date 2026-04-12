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
        private uint _lastAdvertisementCounter = 0;
        private bool _counterInitialized = false;
        private int _totalReceived = 0;
        private int _totalMissed = 0;
        private readonly Queue<bool> _recentWindow = new(); // true=received, false=missed
        private int _windowMissedCount = 0;
        private const int WindowSize = 100;

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

                            // After RemoveDeviceAsync the D-Bus object is gone but the device
                            // can still appear in GetDevicesAsync for a brief moment. Calling
                            // GetAddressAsync on it throws UnknownObject. Skip silently — this
                            // is transient noise, not a real failure worth counting.
                            string? address;
                            try { address = await device.GetAddressAsync(); }
                            catch { continue; }
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
                                // StopDiscoveryAsync may throw if discovery was already stopped
                                // (e.g. HandleDevice stopped it before the exception propagated)
                                try { await adapter.StopDiscoveryAsync().ConfigureAwait(false); } catch { }
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
            // Do NOT stop discovery here — only stop it in GATT mode just before connecting.
            // Stopping it for advertisement mode causes a ~1s scan gap on every detection,
            // which creates a timing race where every other ESP32 wake-up is missed.

            // Check for advertisement mode (timer wake-up): manufacturer data contains weight.
            // BlueZ only exposes the ManufacturerData D-Bus property when the device actually
            // advertised manufacturer-specific data. If it's absent (GATT/power-cycle mode or
            // old firmware), the library throws a DBusException instead of returning null —
            // so we catch it and treat it as "no manufacturer data → fall through to GATT mode".
            IDictionary<ushort, object>? manufacturerData = null;
            try { manufacturerData = await dev.GetManufacturerDataAsync(); } catch { }
            if (manufacturerData != null && manufacturerData.Count > 0)
            {
                var payload = manufacturerData.Values.First() as byte[];
                // BlueZ strips the 2-byte company ID from manufacturer data and uses it as the
                // dictionary key. The value contains only the remaining bytes — in our case the
                // 4-byte float. So check >= 4 and read at offset 0, not >= 6 at offset 2.
                if (payload != null && payload.Length >= 4)
                {
                    float weight = BitConverter.ToSingle(payload, 0);

                    // Counter is in bytes 4-5 (uint16). Track received/missed using a rolling
                    // window so we can report what fraction of recent advertisements were caught.
                    if (payload.Length >= 6)
                    {
                        ushort counter = BitConverter.ToUInt16(payload, 4);
                        if (!_counterInitialized)
                        {
                            _counterInitialized = true;
                            AddToWindow(received: true);
                            _totalReceived++;
                            _logger.LogInformation(
                                "First advertisement received (ESP32 counter={Counter}). Window: {WR}/{WT} received. Total: {TR} received, {TM} missed.",
                                counter, _recentWindow.Count - _windowMissedCount, _recentWindow.Count, _totalReceived, _totalMissed);
                        }
                        else if (counter == (ushort)_lastAdvertisementCounter)
                        {
                            // Duplicate: BlueZ re-reported cached data before RemoveDeviceAsync cleared it.
                            _logger.LogDebug("Duplicate advertisement #{Counter} ignored", counter);
                            try { await adapter.RemoveDeviceAsync(dev.ObjectPath); } catch { }
                            return;
                        }
                        else
                        {
                            ushort missed = (ushort)(counter - (ushort)_lastAdvertisementCounter - 1);
                            for (int m = 0; m < missed; m++) { AddToWindow(received: false); _totalMissed++; }
                            AddToWindow(received: true);
                            _totalReceived++;

                            int windowReceived = _recentWindow.Count - _windowMissedCount;
                            if (missed > 0)
                                _logger.LogWarning(
                                    "Advertisement #{Counter} received — {Missed} MISSED before it. Window: {WR}/{WT} received ({WM} missed). Total: {TR} received, {TM} missed.",
                                    counter, missed, windowReceived, _recentWindow.Count, _windowMissedCount, _totalReceived, _totalMissed);
                            else
                                _logger.LogInformation(
                                    "Advertisement #{Counter} received. Window: {WR}/{WT} received ({WM} missed). Total: {TR} received, {TM} missed.",
                                    counter, windowReceived, _recentWindow.Count, _windowMissedCount, _totalReceived, _totalMissed);
                        }
                        _lastAdvertisementCounter = counter;
                    }

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

                // Remove from BlueZ cache so the next discovery gets fresh scan data.
                // Without this, BlueZ keeps the stale manufacturer data even after the ESP32
                // reboots into GATT mode — causing the gateway to loop forever thinking it's
                // still in advertisement mode.
                try { await adapter.RemoveDeviceAsync(dev.ObjectPath); } catch { }
                return; // Do not connect in advertisement mode
            }

            // GATT mode (power-cycle): stop discovery before connecting
            await adapter.StopDiscoveryAsync();

            _logger.LogInformation("found device {Address}, connecting for configuration...", addr);
            var bleDevice = _serviceProvider.GetService<BleDevice>();
            var alias = await dev.GetAliasAsync();

            bleDevice!.Setup(addr, alias);
            bleDevice.OnDeviceFound(dev);

            dev.Connected += DeviceConnectedAsync;
            dev.Disconnected += DeviceDisconnectedAsync;

            _logger.LogInformation($"Connecting to {addr}...");
            _deviceIsDeisconected = new TaskCompletionSource<bool>();
            await Task.Delay(3000); // Give BCM43438 time to fully transition from scan to connect mode
            try
            {
                await dev.ConnectAsync();
            }
            catch (Exception ex) when (ex.Message.Contains("le-connection-abort-by-local"))
            {
                // The RPi BCM43438 chip (shared WiFi/BT) aborted the connection internally.
                // Power-cycling the adapter via BlueZ clears the stuck HCI state so the
                // next attempt starts clean instead of failing immediately again.
                _logger.LogWarning("BLE connection aborted by local chip, resetting adapter...");
                try
                {
                    await adapter.SetPoweredAsync(false);
                    await Task.Delay(2000);
                    await adapter.SetPoweredAsync(true);
                    await Task.Delay(2000);
                }
                catch (Exception resetEx)
                {
                    _logger.LogError(resetEx, "Failed to reset adapter after le-connection-abort");
                }
                throw; // re-throw so the outer catch handles the failure count
            }
            _consecutiveFailures = 0;

            // Poll for GATT service resolution as a fallback to the ServicesResolved event.
            // The D-Bus PropertiesChanged signal can be missed if it fires during ConnectAsync
            // before the Linux.Bluetooth event subscription processes it. Polling ensures we
            // always handle service resolution regardless of event timing.
            _logger.LogInformation("Polling for GATT service resolution...");
            bool servicesResolved = false;
            for (int i = 0; i < 40; i++) // up to 20 seconds
            {
                try
                {
                    if (await dev.GetServicesResolvedAsync())
                    {
                        servicesResolved = true;
                        break;
                    }
                }
                catch { break; } // device likely disconnected
                await Task.Delay(500);
            }

            if (servicesResolved)
            {
                _logger.LogInformation("GATT services resolved, handling configuration...");
                await bleDevice.HandleServicesResolvedAsync(dev);
            }
            else
            {
                _logger.LogWarning("GATT services did not resolve — device may have disconnected early.");
            }

            _logger.LogInformation("Wait for disconnecting");
            await _deviceIsDeisconected.Task;
        }

        private void AddToWindow(bool received)
        {
            if (_recentWindow.Count == WindowSize)
            {
                bool removed = _recentWindow.Dequeue();
                if (!removed) _windowMissedCount--;
            }
            _recentWindow.Enqueue(received);
            if (!received) _windowMissedCount++;
        }

        private async Task EnsureDiscoveryIsActiveAsync(Adapter adapter)
        {
            if (!await adapter.GetDiscoveringAsync())
            {
                // DuplicateData=true disables BlueZ's per-session dedup filter.
                // Without this, BlueZ only reports a given MAC address once per scan session —
                // so after we RemoveDevice and the ESP32 advertises again in the same session,
                // BlueZ silently drops it. With DuplicateData=true every advertisement is reported.
                // Filter to only our ESP32 service UUID so BlueZ ignores all other BLE devices
                // (phones, etc.) in range. Without this, DuplicateData=true floods the system
                // with D-Bus events for every nearby device, overloading bluetoothd on the RPi
                // BCM43438 chip and causing WiFi drops / SSH loss.
                await adapter.SetDiscoveryFilterAsync(new Dictionary<string, object>
                {
                    { "Transport", "le" },
                    { "DuplicateData", true },
                    { "UUIDs", new string[] { "f0cbd08a-41a1-4b14-b66f-420e6c7f6d1f" } }
                });
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

