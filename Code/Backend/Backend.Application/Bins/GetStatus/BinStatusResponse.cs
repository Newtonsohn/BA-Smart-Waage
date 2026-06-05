using Backend.Application.Inventories.Contracts;

namespace Backend.Application.Bins.GetStatus
{
    public record BinStatusResponse(Guid BinId, string BinName, string MacAddress, bool IsOnline, DateTime LastSeen, BinInventoryResponse Inventory);
}
