using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.Assign
{
    public record AssignBinCommand(Guid BinId, Guid GatewayId): ICommand;
}
