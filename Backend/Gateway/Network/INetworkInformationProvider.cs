using Backend.Domain.Kernel;

namespace EdgeDevice.Network
{
    public interface INetworkInformationProvider
    {
        Result<string> GetMacAddress();
        Result<string> GetIpAddress();
    }
}
