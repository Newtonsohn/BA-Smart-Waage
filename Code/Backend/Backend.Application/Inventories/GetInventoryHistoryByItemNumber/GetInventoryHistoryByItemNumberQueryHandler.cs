using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.GetInventoryHistoryByItemNumber
{
    internal class GetInventoryHistoryByItemNumberQueryHandler(IApplicationDbContext _dbContext, 
        ILogger<GetInventoryHistoryByItemNumberQueryHandler> _logger) 
        : IQueryHandler<GetInventoryHistoryByItemNumberQuery, InventoryItemHistoryEntryResponse>
    {
        public async Task<Result<InventoryItemHistoryEntryResponse>> Handle(GetInventoryHistoryByItemNumberQuery query, CancellationToken cancellationToken)
        {
            try
            {
                var item = await _dbContext.InventoryItems.FirstOrDefaultAsync(i => i.ItemNumber == query.ItemNumber, cancellationToken: cancellationToken);
                if (item is null)
                {
                    return Result.Failure<InventoryItemHistoryEntryResponse>(Error.NullValue);
                }
                var dbQuery = _dbContext.InventoryItemStockChanges.Where(i => i.InventoryItemId == item.Id);


                dbQuery = dbQuery.Where(i => i.TimeStamp <= query.To.ToUniversalTime());
                dbQuery = dbQuery.Where(i => i.TimeStamp >= query.From.ToUniversalTime());
 
                var entries = await dbQuery.OrderBy(i => i.TimeStamp).Take(5000).ToListAsync(cancellationToken);

                if (!entries.Any())
                {
                    dbQuery = dbQuery.Where(i => i.TimeStamp <= query.From.ToUniversalTime());
                    var entry = await dbQuery.OrderBy(i => i.TimeStamp).FirstOrDefaultAsync();
                    if (entry != null)
                    {
                        entries.Add(entry);
                    }
                }
                var historyItems = entries.Select(x => new InventoryItemHistoryEntry(InventoryItemCount.GetCount(x.Weight, item.ItemWeight), x.TimeStamp)).ToList();
                if(!historyItems.Any())
                {
                    return Result.Success(new InventoryItemHistoryEntryResponse(historyItems, historyItems.First().Timestamp, historyItems.Last().Timestamp));
                }
                else
                {
                    return Result.Success(new InventoryItemHistoryEntryResponse(historyItems, query.From, query.To));
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Inventory item history could not be fetched from the database.");
                return Result.Failure<InventoryItemHistoryEntryResponse>(Error.UnexpectedFailure);
            }
        }
    }
}
