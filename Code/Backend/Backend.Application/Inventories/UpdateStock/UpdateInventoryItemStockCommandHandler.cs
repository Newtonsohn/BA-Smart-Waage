using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.UpdateStock
{
    internal class UpdateInventoryItemStockCommandHandler(IInventoryItemStockRepository repository, ILogger<UpdateInventoryItemStockCommandHandler> _logger) : ICommandHandler<UpdateInventoryItemStockCommand>
    {
        public async Task<Result> Handle(UpdateInventoryItemStockCommand command, CancellationToken cancellationToken)
        {
            var lastUpdate = await repository.GetCurrentWeightAsync(command.InventoryItemId);
            var newWeight = lastUpdate + command.WeightChange;
            if (newWeight < 0)
            {
                _logger.LogWarning("New inventory item weight is less then 0: {Weight}", newWeight);
                newWeight = 0;
            }
            await repository.AddWeightChangeAsync(command.InventoryItemId, newWeight, command.TimeStamp);
            return Result.Success();
        }
    }
}
