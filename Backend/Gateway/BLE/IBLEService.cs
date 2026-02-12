using Backend.Domain.Kernel;

namespace EdgeDevice.BLE
{
    internal interface IBLEService
    {
        Task<Result> UpdatedAssignedDevicesAsync(ISet<string> macAddresses);
    }
}
