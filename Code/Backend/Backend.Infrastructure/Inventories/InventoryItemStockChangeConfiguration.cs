using Backend.Domain.Inventories;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.Inventories
{
    internal class InventoryItemStockChangeConfiguration : IEntityTypeConfiguration<InventoryItemStockChange>
    {
        public void Configure(EntityTypeBuilder<InventoryItemStockChange> builder)
        {
            builder.HasKey(x => x.Id);
            builder.Property(x => x.Weight).IsRequired();
            builder.Property(x => x.TimeStamp).IsRequired();
            builder.Property(x => x.InventoryItemId).IsRequired();
        }
    }
}
