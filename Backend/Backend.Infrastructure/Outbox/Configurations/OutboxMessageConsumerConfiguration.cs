using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;

namespace Backend.Infrastructure.Outbox.Configurations
{
    internal class OutboxMessageConsumerConfiguration: IEntityTypeConfiguration<OutboxMessageConsumer>
    {
        public void Configure(EntityTypeBuilder<OutboxMessageConsumer> builder)
        {
            builder.HasKey(x => new
            {
                x.Id,
                x.ConsumerName
            });
        }
    }
}
