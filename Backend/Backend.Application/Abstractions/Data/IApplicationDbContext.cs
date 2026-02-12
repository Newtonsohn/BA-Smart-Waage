using Backend.Domain.Bins;
using Backend.Domain.Gateways;
using Backend.Domain.Inventories;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.ChangeTracking;

namespace Backend.Application.Abstractions.Data;

public interface IApplicationDbContext
{
    DbSet<Gateway> Gateways { get; }
    DbSet<Bin> Bins { get; }
    DbSet<BinWeightUpdate> BinWeightUpdates { get; }

    DbSet<InventoryItem> InventoryItems { get; }
    DbSet<InventoryItemStockChange> InventoryItemStockChanges { get; }

    IEnumerable<EntityEntry<TEntity>> Entries<TEntity>()
         where TEntity : class;
    Task<int> SaveChangesAsync(CancellationToken cancellationToken = default);
}
