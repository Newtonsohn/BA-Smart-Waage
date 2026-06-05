using Backend.Domain.Kernel;
using Backend.Infrastructure.Database;
using Backend.Infrastructure.Outbox;
using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Noser.RAGnRoll.Persistence.Idempotence;
internal class IdempotenceDomainEventHandler<TDomainEvent>(INotificationHandler<TDomainEvent> _decorated, ApplicationDbContext dbContext, ILogger<IdempotenceDomainEventHandler<TDomainEvent>> _logger) : INotificationHandler<TDomainEvent>
    where TDomainEvent : IDomainEvent
{
    public async Task Handle(TDomainEvent notification, CancellationToken cancellationToken)
    {
        var consumer = _decorated.GetType().Name;
        if (await dbContext.OutboxMessageConsumers.AnyAsync(c => c.Id == notification.Id && c.ConsumerName == consumer))
            return;

        await _decorated.Handle(notification, cancellationToken);

        dbContext.OutboxMessageConsumers.Add(new OutboxMessageConsumer()
        {
            Id = notification.Id,
            ConsumerName = consumer,
        });
        try
        {
            await dbContext.SaveChangesAsync();
        }
        catch {
            var domainEventType = notification.GetType().Name;
            _logger.LogError($"OutboxMessageConsumer [{notification.Id},{domainEventType}, {consumer}] could not be added to the database.");
            throw;
        }
    }
}
