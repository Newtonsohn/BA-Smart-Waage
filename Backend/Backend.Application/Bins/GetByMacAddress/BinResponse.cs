using Backend.Domain.Bins;

namespace Backend.Application.Bins.GetByMacAddress
{
    public record BinResponse(
        Guid Id,
        string Name,
        double Treshold,
        double Capacity, 
        int UpdateInterval,
        int HeartBeatInterval,
        InventoryItemResponse? InventoryItem
        )
    {
        public BinResponse(Bin bin)
    : this(
        bin.Id,
        bin.DeviceName,
        bin.Treshold,
        bin.Capacity,
        bin.UpdateInterval,
        bin.HeartbeatInterval,
        bin.InventoryItem is not null ? new InventoryItemResponse(bin.InventoryItem.Id, bin.InventoryItem.ItemName, bin.InventoryItem.ItemNumber, bin.InventoryItem.ItemWeight) :null
          )
        { }
    }
    public record InventoryItemResponse(Guid Id, string ItemName, string ItemNumber, double ItemWeight);
}
