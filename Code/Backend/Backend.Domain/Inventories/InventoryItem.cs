namespace Backend.Domain.Inventories
{
    public class InventoryItem
    {
        public Guid Id { get; init; } = Guid.CreateVersion7();
        public required string ItemName { get; set; }
        public required string ItemNumber { get; set; }
        /// <summary>
        /// Weight in grams
        /// </summary>
        public required double ItemWeight { get; set; }
        /// <summary>
        /// Treshold value in number of items if InventoryItem.Indicator == Quantity else Treshold in %
        /// </summary>
        public required double Treshold { get; set; }
        public StockIndicator Indicator { get; set; }
    }

    public static class InventoryItemCount
    {
        public static int GetCount(double Weight, double ItemWeight)
        {
            return (int)Math.Round(Weight / ItemWeight, 0, MidpointRounding.AwayFromZero);
        }
    }
}
