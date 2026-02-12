namespace Backend.Domain.Inventories
{
    public interface IInventoryItemStockRepository
    {
        Task<double> GetCurrentWeightAsync(Guid inventoryItemId);
        Task AddWeightChangeAsync(Guid inventoryItemId, double weight, DateTime timeStamp);
    }
}
