using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.Register
{
    public record RegisterBinCommand(string DeviceName, string MacAddress, Guid GatewayIdToAssign): ICommand<Guid>;
}
