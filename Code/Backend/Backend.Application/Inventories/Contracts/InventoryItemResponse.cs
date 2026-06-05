using Backend.Domain.Inventories;

namespace Backend.Application.Inventories.Contracts
{
    public record InventoryItemResponse(Guid Id, string ItemNumber, string ItemName, double ItemWeight, double Treshold, StockIndicator StockIndicator);
}
