using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Gateways.Register
{
    /// <summary>
    /// Registers a new device or updates an existing device. True indicates a new device was created. False indicates an existings device was udpated.
    /// </summary>
    /// <param name="MacAddress"></param>
    /// <param name="IpAddress"></param>
    public record RegisterGatewayCommand(string MacAddress, string IpAddress): ICommand<bool>;
}
