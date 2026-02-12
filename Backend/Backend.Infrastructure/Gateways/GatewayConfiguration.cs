using Backend.Domain.Gateways;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.Gateways;

internal sealed class GatewayConfiguration : IEntityTypeConfiguration<Gateway>
{
    public void Configure(EntityTypeBuilder<Gateway> builder)
    {
        builder.HasKey(x => x.Id);
        builder.HasIndex(x => x.IpAddress).IsUnique();
        builder.HasIndex(x => x.MacAddress).IsUnique();

        builder.HasMany(x => x.Bins).WithOne().HasForeignKey(x => x.GatewayId);
    }
}
