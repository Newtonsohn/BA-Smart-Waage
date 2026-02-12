using Backend.Application.Abstractions.Data;
using Backend.Domain.Bins;
using Backend.Domain.Bins.Events;
using MediatR;
using Microsoft.Extensions.DependencyInjection;

namespace Backend.Application.Bins.UpdateWeight
{
    internal class BinWeightChangedDomainEventHandler(IServiceScopeFactory _scopeFactory) : INotificationHandler<BinWeightChangedDomainEvent>
    {
        public async Task Handle(BinWeightChangedDomainEvent notification, CancellationToken cancellationToken)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext= scope.ServiceProvider.GetRequiredService<IApplicationDbContext>();

            var weightUpdate = new BinWeightUpdate()
            {
                BinId = notification.BinId,
                TimeStamp = notification.Timestamp,
                Weight = notification.CurrentWeight,
            };

            dbContext.BinWeightUpdates.Add(weightUpdate);
            await dbContext.SaveChangesAsync(cancellationToken);
        }
    }
}
