using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Inventories.UpdateStock
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="InventoryItemId"></param>
    /// <param name="WeightChange">Weight change in grams</param>
    /// <param name="TimeStamp"></param>
    public record UpdateInventoryItemStockCommand(Guid InventoryItemId, double WeightChange, DateTime TimeStamp): ICommand;
}
