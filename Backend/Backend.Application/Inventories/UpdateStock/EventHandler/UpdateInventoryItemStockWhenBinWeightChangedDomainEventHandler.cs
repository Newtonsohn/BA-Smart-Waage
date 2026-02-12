using Backend.Domain.Bins.Events;
using Backend.Domain.Kernel;
using MediatR;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.UpdateStock.EventHandler
{
    internal class UpdateInventoryItemStockWhenBinWeightChangedDomainEventHandler(IServiceScopeFactory _serviceScopeFactory, ILogger<UpdateInventoryItemStockWhenBinWeightChangedDomainEventHandler> _logger) 
        : INotificationHandler<BinWeightChangedDomainEvent>
    {
        public async Task Handle(BinWeightChangedDomainEvent notification, CancellationToken cancellationToken)
        {
            using var scope = _serviceScopeFactory.CreateScope(); 
            var sender = scope.ServiceProvider.GetRequiredService<ISender>();

            var diff = notification.CurrentWeight - notification.OldWeight;
            var command = new UpdateInventoryItemStockCommand(notification.InventoryItemId, notification.CurrentWeight - notification.OldWeight, notification.Timestamp);
            var result = await sender.Send(command);
            result.Match(
                onSuccess: () => { },
                onFailure: (error) =>
                {
                    _logger.LogError("An error occured during updating inventory item stock: {Error}", error);
                    if (error == Error.UnexpectedFailure)
                    {
                        throw new InvalidOperationException("An error occured during updating inventory item stock");
                    }
                });
        }
    }
}
