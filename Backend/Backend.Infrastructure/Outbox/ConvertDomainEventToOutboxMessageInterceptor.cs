using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Newtonsoft.Json;

namespace Backend.Infrastructure.Outbox;
public class ConvertDomainEventToOutboxMessageInterceptor: SaveChangesInterceptor
{
    private static readonly JsonSerializerSettings Settings = new()
    {
        TypeNameHandling = TypeNameHandling.All,
    };


    public override async ValueTask<InterceptionResult<int>> SavingChangesAsync(DbContextEventData eventData, InterceptionResult<int> result, CancellationToken cancellationToken = default)
    {
        DbContext? dbContext = eventData.Context;
        if (dbContext == null)
        {
            return await base.SavingChangesAsync(eventData, result, cancellationToken);
        }

        var domainEvents = dbContext.ChangeTracker.Entries<Aggregate>()
            .Select(e => e.Entity)
            .SelectMany(e =>
            {
                var domainEvents = e.DomainEvents;
                e.ClearDomainEvents();
                return domainEvents;
            })
            .ToList();

        var outboxMessages = domainEvents.Select(e => new OutboxMessage
        {
            Content = JsonConvert.SerializeObject(e, Settings),
            Type = e.GetType().Name,
            OccurredOnUtc = DateTime.UtcNow,
            Id = Guid.CreateVersion7()
        }).ToList();

        dbContext.Set<OutboxMessage>().AddRange(outboxMessages);
        return await base.SavingChangesAsync(eventData, result, cancellationToken);
    }
}
