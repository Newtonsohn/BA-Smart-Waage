using Backend.Domain.Kernel;

namespace Backend.Domain.Bins.Events
{
    public record InventoryItemAssignedDomainEvent(Guid BinId, Guid NewItemId, Guid? LastItemId):DomainEvent;
}
