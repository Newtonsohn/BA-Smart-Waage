using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Kernel;
using MediatR;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Inventories.GetInventoryItems
{
    internal class GetItemNumbersQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetInventoryItemsQuery, IReadOnlyList<InventoryItemResponse>>
    {
        public async Task<Result<IReadOnlyList<InventoryItemResponse>>> Handle(GetInventoryItemsQuery request, CancellationToken cancellationToken)
        {
            return await _dbContext.InventoryItems.Select(i => new InventoryItemResponse(i.Id, i.ItemName, i.ItemNumber, i.Indicator)).ToListAsync(cancellationToken);
        }
    }
}
