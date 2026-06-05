using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Inventories.Update
{
    class UpdateInventoryItemCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<UpdateInventoryItemCommand>
    {
        public async Task<Result> Handle(UpdateInventoryItemCommand command, CancellationToken cancellationToken)
        {
            var item = await _dbContext.InventoryItems.FirstOrDefaultAsync(i => i.Id == command.InventoryItemId, cancellationToken);
            if (item == null)
            {
                return Result.Failure(Error.NullValue);
            }

            item.ItemNumber =command.ItemNumber;
            item.ItemWeight = command.ItemWeight;
            item.Indicator = command.Indicator; 
            item.Treshold = command.Treshold;
            item.ItemName = command.ItemName;
            
            _dbContext.InventoryItems.Update(item);
            await _dbContext.SaveChangesAsync();
            return Result.Success();
        }
    }
}
