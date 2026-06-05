using Backend.Infrastructure.Database;
using Hangfire;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Infrastructure.Outbox.Jobs;

public interface ICleanUpOutboxFoodPrintJob
{
    Task CleanUp();
}

[AutomaticRetry(Attempts = 1)]
internal class CleanUpOutboxFoodPrintJob(ApplicationDbContext _dbContext, ILogger<CleanUpOutboxFoodPrintJob> _logger) : ICleanUpOutboxFoodPrintJob
{
    public async Task CleanUp()
    {
        try
        {
            var removedEntitiesCount = await _dbContext.OutboxMessages.Where(m => m.ProcessedOnUtc != null)
          .ExecuteDeleteAsync();
            _logger.LogInformation($"Cleanup job removed {removedEntitiesCount} processed outbox messages.");

            var existingOutboxMessageIds = await _dbContext.OutboxMessages.Select(m => m.Id).ToListAsync();
            removedEntitiesCount = await _dbContext.OutboxMessageConsumers.Where(c => !existingOutboxMessageIds.Contains(c.Id))
                .ExecuteDeleteAsync();

            _logger.LogInformation($"Cleanup job removed {removedEntitiesCount} outbox messages consumer.");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Unexpected error uccurred durign clean up job.");
        }
    }
}
