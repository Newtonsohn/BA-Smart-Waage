using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;

namespace Backend.Application.Inventories.Update
{
    /// <summary>
    /// Command to update an inventory item.
    /// </summary>
    /// <param name="InventoryItemId"></param>
    /// <param name="ItemNumber"></param>
    /// <param name="ItemName"></param>
    /// <param name="ItemWeight">Weight in grams</param>
    /// <param name="Treshold">Treshold value in number of items if the Indicator == StockIndicator.Quantity else in %</param>
    /// <param name="Indicator"></param>
    public record UpdateInventoryItemCommand(Guid InventoryItemId, 
        string ItemNumber,
        string ItemName,
        double ItemWeight,
        double Treshold,
        StockIndicator Indicator
        ): ICommand;
}
