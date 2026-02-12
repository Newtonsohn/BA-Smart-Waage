namespace Backend.Domain.Inventories
{
    public class InventoryItemStockChange
    {
            public Guid Id { get; init; } = Guid.CreateVersion7();
            public double Weight { get; init; }
            public DateTime TimeStamp { get; init; }
            public Guid InventoryItemId { get; init; }
    }
}
