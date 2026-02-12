using Backend.Application.Abstractions.Data;
using Backend.Domain.Bins.Events;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.UpdateStock.EventHandler
{
    internal class UpdateInventoryItemStockWhenInventoryItemAssignedDomainEventHandler(IServiceScopeFactory _scopeFactory,
        ILogger<UpdateInventoryItemStockWhenInventoryItemAssignedDomainEventHandler> _logger) 
        : INotificationHandler<InventoryItemAssignedDomainEvent>
    {
        public async Task Handle(InventoryItemAssignedDomainEvent notification, CancellationToken cancellationToken)
        {
            using var scope = _scopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<IApplicationDbContext>();
            var sender = scope.ServiceProvider.GetRequiredService<ISender>();
            var inventoryStockRepository = scope.ServiceProvider.GetRequiredService<IInventoryItemStockRepository>();

            var bin = await dbContext.Bins.FirstOrDefaultAsync(b => b.Id == notification.BinId);
            var updateTime = DateTime.UtcNow;

            if (notification.LastItemId.HasValue)
            {
                var updateOldInventoryItemCommand = new UpdateInventoryItemStockCommand(notification.LastItemId.Value, -bin!.CurrentWeight, updateTime);
                var updateOldInventoryItemCommandResult = await sender.Send(updateOldInventoryItemCommand, cancellationToken);
                updateOldInventoryItemCommandResult.Match(
                    onSuccess: () => { },
                    onFailure: (error) => { _logger.LogError("Inventory stock could not be updated: {Error}", error);
                    });
            }
            var command = new UpdateInventoryItemStockCommand(notification.NewItemId, bin!.CurrentWeight, updateTime);
            var result = await sender.Send(command, cancellationToken);
            result.Match(
                onSuccess: () => { },
                onFailure: (error) => {
                    _logger.LogError("Inventory stock could not be updated: {Error}", error);
                });
        }
    }
}
