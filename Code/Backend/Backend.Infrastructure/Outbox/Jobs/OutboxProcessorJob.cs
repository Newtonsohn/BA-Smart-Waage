using Backend.Domain.Kernel;
using Backend.Infrastructure.Database;
using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;

namespace Backend.Infrastructure.Outbox.Jobs;

public interface IOutboxProcessorJob
{
    public Task ProcessAsync(CancellationToken cancellationToken);
}

public class OutboxProcessorJob(ApplicationDbContext _dbContext, IPublisher _publisher, ILogger<OutboxProcessorJob> _logger) : IOutboxProcessorJob
{
    public static JsonSerializerSettings JsonSerializerSettings = new JsonSerializerSettings()
    {
        TypeNameHandling = TypeNameHandling.All,
    };

    public async Task ProcessAsync(CancellationToken cancellationToken)
    {
        using var transaction = await _dbContext.Database.BeginTransactionAsync(cancellationToken);
        List<OutboxMessage> messages = await GetOutboxMessages(_dbContext, cancellationToken);
        var ids = messages.Select(x => x.Id).ToList();

        foreach (var message in messages)
        {
            Exception? exception = null;
            try
            {
                var domainEvent = JsonConvert.DeserializeObject<IDomainEvent>(message.Content, JsonSerializerSettings)!;
                await _publisher.Publish(domainEvent, cancellationToken);
            }
            catch (Exception ex)
            {
                _logger.LogError(
                    ex,
                    $"Error while publishing outbox message {message.Id}");
                exception = ex;
            }

            await UpdateOutboxMessageAsync(_dbContext, message, exception);
        }

        await transaction.CommitAsync();
    }

    private static async Task<List<OutboxMessage>> GetOutboxMessages(ApplicationDbContext _dbContext, CancellationToken cancellationToken)
    {
        int batchSize = 10; // Adjust batch size as needed

        var messages = await _dbContext.OutboxMessages
            .FromSqlRaw(@"
            SELECT *
            FROM public.""OutboxMessages"" 
            WHERE ""ProcessedOnUtc"" IS NULL 
            ORDER BY ""OccurredOnUtc"" 
            LIMIT {0}
            FOR UPDATE SKIP LOCKED", batchSize)
            .ToListAsync(cancellationToken);

        return messages;
    }

    private async Task UpdateOutboxMessageAsync(ApplicationDbContext dbContext, OutboxMessage message, Exception? exception)
    {
        if (exception is null)
        {
            message.ProcessedOnUtc = DateTime.UtcNow;
        }
        else
        {
            message.Retries++;
            message.Error = exception.ToString();
            if (message.Retries > 3)
            {
                _logger.LogError(exception, "Publishing OutboxMessage failed more than three times");
            }
        }
        dbContext.OutboxMessages.Update(message);
        await dbContext.SaveChangesAsync();

    }
}