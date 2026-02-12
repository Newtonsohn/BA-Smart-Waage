using Backend.Domain.Bins;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.Bins;

internal sealed class BinConfiguration : IEntityTypeConfiguration<Bin>
{
    public void Configure(EntityTypeBuilder<Bin> builder)
    {
        builder.HasKey(x => x.Id);
        builder.Property(x => x.DeviceName).IsRequired();
        builder.HasIndex(x => x.MacAddress).IsUnique();
        builder.Property(x => x.DeviceName).IsRequired();


        builder.HasOne(b => b.InventoryItem)
        .WithMany()
        .HasForeignKey(b => b.InventoryItemId)
        .OnDelete(DeleteBehavior.SetNull);

        builder
        .Navigation(b => b.InventoryItem)
        .AutoInclude();
    }
}
