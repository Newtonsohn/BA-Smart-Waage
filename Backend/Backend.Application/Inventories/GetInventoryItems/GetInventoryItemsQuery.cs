using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;

namespace Backend.Application.Inventories.GetInventoryItems
{
    public record InventoryItemResponse(Guid InventoryItemId, string ItemName, string ItemNumber, StockIndicator StockIndicator);
    public record GetInventoryItemsQuery(): IQuery<IReadOnlyList<InventoryItemResponse>>;
}
