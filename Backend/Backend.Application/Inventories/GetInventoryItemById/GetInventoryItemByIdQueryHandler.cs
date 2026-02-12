using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Inventories.GetInventoryItemById
{
    internal class GetInventoryItemByIdQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetInventoryItemByIdQuery, InventoryItemResponse>
    {
        public async Task<Result<InventoryItemResponse>> Handle(GetInventoryItemByIdQuery query, CancellationToken cancellationToken)
        {
            var item = await _dbContext.InventoryItems.FirstOrDefaultAsync(i => i.Id == query.Id, cancellationToken);
            return item is null ? Result.Failure<InventoryItemResponse>(Error.NullValue) : Result.Success(new InventoryItemResponse(item.Id, item.ItemNumber, item.ItemName, item.ItemWeight, item.Treshold, item.Indicator));
        }
    }
}
