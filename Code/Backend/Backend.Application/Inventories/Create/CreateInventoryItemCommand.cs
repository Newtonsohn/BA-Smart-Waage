using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;

namespace Backend.Application.Inventories.Create
{
    /// <summary>
    /// Command to create an inventory item.
    /// </summary>
    /// <param name="ItemName"></param>
    /// <param name="ItemNumber"></param>
    /// <param name="ItemWeight">Weight of the item in grams</param>
    /// <param name="Treshold">Treshold value in number of items if the Indicator == StockIndicator.Quantity else in %.</param>
    /// <param name="Indicator"></param>
    public record CreateInventoryItemCommand(
        string ItemName,
        string ItemNumber,
        double ItemWeight,
        double Treshold,
        StockIndicator Indicator): ICommand<Guid>;
}
