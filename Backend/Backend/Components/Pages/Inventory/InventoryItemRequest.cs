using Backend.Domain.Inventories;

namespace Backend.API.Components.Pages.Inventory
{
    public class InventoryItemRequest
    {
        public Guid Id;
        public string ItemName = string.Empty;
        public string ItemNumber = string.Empty;
        public double Treshold;
        public double ItemWeight;
        public StockIndicator Indicator;
    }
}
