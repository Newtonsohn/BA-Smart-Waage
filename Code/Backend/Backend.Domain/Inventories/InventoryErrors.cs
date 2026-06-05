using Backend.Domain.Kernel;

namespace Backend.Domain.Inventories
{
    public static class InventoryErrors
    {
        public static Error Duplicate(string itemNumber) => Error.Conflict(
             "Inventory.Duplicate",
           $"It exists already an item with the item number: {itemNumber}");
        public static Error ItemNotFound(string itemNumber) => Error.NotFound(
           "Inventory.ArticelNotFound",
           $"There is item registerd for item number: {itemNumber}");

        public static Error BinNotFound(Guid binId) => Error.NotFound(
          "Inventory.BinNotFound",
          $"No Bin found with this id: {binId}");

        public static Error BinNotFoundByMac(string macAddress) => Error.NotFound(
          "Inventory.MacNotFound",
          $"No Bin found with MAC address: {macAddress}");

        public static Error BinNotConfigured => Error.NotFound(
          "Inventory.BinNotConfigured",
          $"Bin is not configured yet.");

        public static Error BinWasNotFilledUp => Error.NotFound(
          "Inventory.BinWasNotFilledUp",
          $"Bin was never filled up.");

        public static Error NegativeItemCount => Error.Conflict(
          "Inventory.NegativeItemCount",
          $"Item Count can not be nagative.");

    }
}
