using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.AssignInventoryItem
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="BinId"></param>
    /// <param name="InventoryItemId"></param>
    /// <param name="Treshold">Treshold value in number of items if the Indicator of the assigned Inventory item == Quantity else in % </param>
    /// <param name="Capacity">Max item count</param>
    public record AssignInventoryItemCommand(Guid BinId, Guid InventoryItemId, double Treshold, int Capacity) : ICommand;
}
