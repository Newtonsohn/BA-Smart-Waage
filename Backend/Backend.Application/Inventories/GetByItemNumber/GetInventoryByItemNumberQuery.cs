using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;

namespace Backend.Application.Inventories.GetByItemNumber
{

    public record InventoryResponse(string ItemNumber, double TotalWeight, int ItemCount, double InventoryFillLevel, List<BinInventoryResponse> Bins);
    public record GetInventoryByItemNumberQuery(string ItemNumber ,bool ShowDetails): IQuery<InventoryResponse>;
}
