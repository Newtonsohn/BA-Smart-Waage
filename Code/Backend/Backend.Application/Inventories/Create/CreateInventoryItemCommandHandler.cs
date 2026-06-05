using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Inventories.Create
{
    internal class CreateInventoryItemCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<CreateInventoryItemCommand, Guid>
    {
        public async Task<Result<Guid>> Handle(CreateInventoryItemCommand command, CancellationToken cancellationToken)
        {
            if(await _dbContext.InventoryItems.AnyAsync(x => x.ItemNumber == command.ItemNumber, cancellationToken))
            {
                return Result.Failure<Guid>(InventoryErrors.Duplicate(command.ItemNumber));
            }
            var item = new InventoryItem()
            {
                ItemName = command.ItemName,
                ItemNumber = command.ItemNumber,
                Treshold = command.Treshold,
                ItemWeight = command.ItemWeight,
                Indicator = command.Indicator,
            };
            _dbContext.InventoryItems.Add(item);
            await _dbContext.SaveChangesAsync();
            return Result.Success(item.Id);
        }
    }
}
