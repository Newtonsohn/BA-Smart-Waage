using Backend.Application.Abstractions.Data;
using Backend.Domain.Bins;
using Backend.Domain.Gateways;
using Backend.Domain.Inventories;
using Backend.Infrastructure.Outbox;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.ChangeTracking;

namespace Backend.Infrastructure.Database
{
    public class ApplicationDbContext(DbContextOptions<ApplicationDbContext> options) : DbContext(options), IApplicationDbContext
    {
        public DbSet<Gateway> Gateways { get; set; }

        public DbSet<Bin> Bins { get; set; }
        public DbSet<BinWeightUpdate> BinWeightUpdates { get; set; }

        public DbSet<OutboxMessage> OutboxMessages { get; set; }
        public DbSet<OutboxMessageConsumer> OutboxMessageConsumers { get; set; }

        public DbSet<InventoryItem> InventoryItems { get; set; }
        public DbSet<InventoryItemStockChange> InventoryItemStockChanges { get; set; }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            modelBuilder.ApplyConfigurationsFromAssembly(typeof(ApplicationDbContext).Assembly);
        }

        public IEnumerable<EntityEntry<TEntity>> Entries<TEntity>() where TEntity : class
        {
            ChangeTracker.DetectChanges();
            return ChangeTracker.Entries<TEntity>();
        }
    }
}
