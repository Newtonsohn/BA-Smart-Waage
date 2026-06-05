using Backend.Domain.Bins;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.SmartBins
{
    internal class BinWeightUpdateConfiguration : IEntityTypeConfiguration<BinWeightUpdate>
    {
        public void Configure(EntityTypeBuilder<BinWeightUpdate> builder)
        {
            builder.HasKey(x => x.Id);
            builder.Property(x => x.Weight).IsRequired();
            builder.Property(x => x.TimeStamp).IsRequired();
            builder.Property(x => x.BinId).IsRequired();
        }
    }
}
