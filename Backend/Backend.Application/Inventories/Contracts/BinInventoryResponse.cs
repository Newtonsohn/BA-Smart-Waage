using Backend.Domain.Bins;
using Backend.Domain.Inventories;

namespace Backend.Application.Inventories.Contracts
{
    public record BinInventoryResponse
    {
        public BinInventoryResponse(Bin bin)
        {
            BinName = bin.DeviceName;
            ItemCount = bin.ItemCount;
            ArgumentNullException.ThrowIfNull(bin.InventoryItem);
            ItemNumber = bin.InventoryItem.ItemNumber;
            ItemName = bin.InventoryItem.ItemName;
            ItemWeight = bin.InventoryItem.ItemWeight;
            CurrentWeight = bin.CurrentWeight;
            FillLevel = bin.FillLevel;
            Indicator = bin.InventoryItem.Indicator;
            IsEmpty = bin.IsEmpty();
        }
        public string BinName { get; init; }
        public string ItemName { get; init; }
        public string ItemNumber { get; init; }
        /// <summary>
        /// Current Weight of the bin in grams
        /// </summary>
        public double CurrentWeight { get; init; }
        public int ItemCount { get; init; }
        /// <summary>
        /// Item Weight in grams
        /// </summary>
        public double ItemWeight { get; init; } 
        /// <summary>
        /// Fill level in %
        /// </summary>
        public double FillLevel { get; init; }
        public StockIndicator Indicator { get; init; }
        /// <summary>
        /// /// Indicates whether the bin is considered empty, as determined by <c>Bin.IsEmpty()</c>.
        /// </summary>
        public bool IsEmpty { get; init; }
    }
}
