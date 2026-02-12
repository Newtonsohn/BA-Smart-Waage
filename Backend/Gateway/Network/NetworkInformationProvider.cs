using Backend.Domain.Kernel;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Net;

namespace EdgeDevice.Network
{
    internal class NetworkInformationProvider(ILogger<NetworkInformationProvider> _logger, IConfiguration _configuration) : INetworkInformationProvider
    {
        private readonly string EthernetInterfaceName = _configuration.GetSection("Network:Interface").Value?? "eth0";
        public Result<string> GetIpAddress()
        {
            try
            {
                var eth0 = NetworkInterface.GetAllNetworkInterfaces()
                    .FirstOrDefault(n => n.Name.Equals(EthernetInterfaceName, StringComparison.OrdinalIgnoreCase));

                if (eth0 == null)
                {
                    _logger.LogError("Network interface not found");
                    return Result.Failure<string>(Error.NotFound(
                        "NetworkInformationProvider.NetworkInterfaceNotFound",
                        "Network interface not found"
                        ));
                }
                    

                var ip = eth0.GetIPProperties().UnicastAddresses
                    .FirstOrDefault(a => a.Address.AddressFamily == AddressFamily.InterNetwork &&
                                         !IPAddress.IsLoopback(a.Address));

                if (ip == null)
                {
                    _logger.LogError("No IPv4 address assigned to interface {NetworkInterface}", EthernetInterfaceName);
                    return Result.Failure<string>(Error.NotFound(
                       "NetworkInformationProvider.IpAddressNotAssigned",
                       "No IPv4 address assigned"
                       ));
                }

                return Result.Success(ip.Address.ToString());
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving IP address");
                throw;
            }
        }

        public Result<string> GetMacAddress()
        {
            try
            {
                var eth0 = NetworkInterface.GetAllNetworkInterfaces()
                    .FirstOrDefault(n => n.Name.Equals(EthernetInterfaceName, StringComparison.OrdinalIgnoreCase));

                if (eth0 == null)
                {
                    _logger.LogError("Network interface not found");
                    return Result.Failure<string>(Error.NotFound(
                        "NetworkInformationProvider.NetworkInterfaceNotFound",
                        "Network interface not found"
                        ));
                }

                var mac = BitConverter.ToString(eth0.GetPhysicalAddress().GetAddressBytes());
                return Result.Success(mac);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error while obtaining MAC address");
                throw;
            }
        }
    }
}
