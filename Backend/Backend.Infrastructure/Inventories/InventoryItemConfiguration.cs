using Backend.Domain.Inventories;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.Inventories
{
    class InventoryItemConfiguration : IEntityTypeConfiguration<InventoryItem>
    {
        public void Configure(EntityTypeBuilder<InventoryItem> builder)
        {
            builder.HasKey(x => x.Id);  
            builder.Property(x => x.ItemNumber).IsRequired();
            builder.Property(x => x.ItemName).IsRequired();
            builder.Property(x => x.ItemWeight).IsRequired();
            builder.Property(x => x.Treshold).IsRequired();
        }
    }
}
