using Backend.Domain.Kernel;

namespace Backend.Domain.Bins.Events
{
    public record BinAssignedDomainEvent(
        Guid BinId, 
        Guid NewGatewayId,
        Guid OldGatewayId): DomainEvent;
}
