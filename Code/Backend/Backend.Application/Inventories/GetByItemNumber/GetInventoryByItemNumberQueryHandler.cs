using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.GetByItemNumber
{
    public class GetInventoryByItemNumberQueryHandler(
        IApplicationDbContext _dbContext,
        ILogger<GetInventoryByItemNumberQueryHandler> _logger) : IQueryHandler<GetInventoryByItemNumberQuery, InventoryResponse>
    {
        public async Task<Result<InventoryResponse>> Handle(GetInventoryByItemNumberQuery query, CancellationToken cancellationToken)
        {

#pragma warning disable CS8602 // Dereference of a possibly null reference.
            var affectedBins = await _dbContext.Bins
                .Where(s => s.InventoryItem.ItemNumber == query.ItemNumber).ToListAsync(cancellationToken: cancellationToken);
#pragma warning restore CS8602 // Dereference of a possibly null reference.

            if (!affectedBins.Any())
            {
                _logger.LogError("There is no item registered for item number: {ItemNumber}", query.ItemNumber);
                return Result.Failure<InventoryResponse>(InventoryErrors.ItemNotFound(query.ItemNumber));
            }

            var inventoryItem = affectedBins.First().InventoryItem;

            var weight = affectedBins.Sum(b => b.CurrentWeight);
            var itemCount = affectedBins.Sum(b => b.ItemCount);
            var capacity = affectedBins.Sum(b => b.Capacity);
            var inventoryItemFillLevel = (weight / capacity) * 100;

            if (query.ShowDetails)
            {
                var binInventories = affectedBins.Select(b => new BinInventoryResponse(b)).ToList();
                var response = new InventoryResponse(query.ItemNumber, weight, itemCount, inventoryItemFillLevel, binInventories);
                return Result.Success(response);
            }
            else
            {
                var response = new InventoryResponse(query.ItemNumber, weight, itemCount, inventoryItemFillLevel, []);
                return Result.Success(response);
            }
        }
    }
}
