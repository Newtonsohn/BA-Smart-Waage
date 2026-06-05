using Backend.Application.Abstractions.Data;
using Backend.Domain.Inventories;
using Microsoft.EntityFrameworkCore;

namespace Backend.Infrastructure.Inventories
{
    internal class InventoryItemStockRepository(IApplicationDbContext _dbContext) : IInventoryItemStockRepository
    {
        public async Task AddWeightChangeAsync(Guid inventoryItemId, double weight, DateTime timeStamp)
        {
            var change = new InventoryItemStockChange()
            {
                Weight = weight,
                InventoryItemId = inventoryItemId,
                TimeStamp = timeStamp
            };
            _dbContext.InventoryItemStockChanges.Add(change);
            await _dbContext.SaveChangesAsync();
        }

        public async Task<double> GetCurrentWeightAsync(Guid inventoryItemId)
        {
            var lastUpdate = await _dbContext.InventoryItemStockChanges.Where(i => i.InventoryItemId == inventoryItemId)
                                                    .OrderByDescending(i => i.TimeStamp)
                                                    .FirstOrDefaultAsync();
            return lastUpdate is null ? 0.0 : lastUpdate.Weight;    
        }
    }
}
