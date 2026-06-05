using Backend.Domain.Kernel;

namespace Backend.Domain.Gateways
{
    public class GatewayErrors
    {
        public static Error AlreadyRegistered(string ipAddress) => Error.Conflict(
           "Gateway.IpAddressAlreadyRegistered",
           $"The gateway with ip address = '{ipAddress}' is already registered.");

        public static Error NotFound(string macAddress) => Error.NotFound(
            "Gateway.GatewayByMacAddressNotFound",
            $"The gateway with MAC-Address; {macAddress} could not be found");
    }
}
