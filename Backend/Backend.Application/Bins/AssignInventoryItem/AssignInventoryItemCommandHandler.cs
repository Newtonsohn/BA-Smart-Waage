using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Bins;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.AssignInventoryItem
{
    internal class AssignInventoryItemCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<AssignInventoryItemCommand>
    {
        public async Task<Result> Handle(AssignInventoryItemCommand command, CancellationToken cancellationToken)
        {
            var bin = await _dbContext.Bins.FirstOrDefaultAsync(s => s.Id == command.BinId, cancellationToken);
            if (bin is null)
            {
                return Result.Failure(BinErrors.NotFound(command.BinId));
            }
            var inventoryItem = await _dbContext.InventoryItems.FirstOrDefaultAsync(s => s.Id == command.InventoryItemId, cancellationToken);

            bin.AssignInventoryItem(inventoryItem!, command.Treshold, command.Capacity);

            _dbContext.Bins.Update(bin);
            await _dbContext.SaveChangesAsync();
            return Result.Success();
        }
    }
}
