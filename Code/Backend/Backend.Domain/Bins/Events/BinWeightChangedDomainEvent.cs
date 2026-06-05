using Backend.Domain.Kernel;

namespace Backend.Domain.Bins.Events
{
    public record BinWeightChangedDomainEvent(Guid BinId, string Name, Guid InventoryItemId, double CurrentWeight, double OldWeight, double ItemWeight, int ItemCount, int ItemCountChange, DateTime Timestamp):DomainEvent;
}
