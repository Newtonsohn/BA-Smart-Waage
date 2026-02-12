using Backend.Application.Abstractions.Data;
using Backend.Domain.Bins.Events;
using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Gateways.EventHandlers
{
    internal class NotifiyGatewayWhenBinRegisteredDomainEventHandler(
        IHttpClientFactory _httpClientFactory,
        ILogger<NotifyGatewaysWhenSmartBinWasAssigned> _logger,
        IServiceScopeFactory _serviceScopeFactory) : NotifiyGatewayDomainEventHandler, INotificationHandler<BinDeviceRegisteredDomainEvent>
    {
        public async Task Handle(BinDeviceRegisteredDomainEvent notification, CancellationToken cancellationToken)
        {
            using var scope = _serviceScopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<IApplicationDbContext>();
            var configuration = scope.ServiceProvider.GetRequiredService<IConfiguration>();

            var gateway = await dbContext.Gateways.Include(g => g.Bins).FirstOrDefaultAsync(g => g.Id == notification.GatewayId, cancellationToken);
            var client = _httpClientFactory.CreateClient("GatewayClient");

            var requestUrl= GetGatewayUpdateUrl(gateway!.IpAddress, configuration);

            await UpdateGateway(gateway, client, requestUrl);

            _logger.LogInformation(
                        "Affected Gateway [{OldGateway}] was updated for bin: {Bin}",
                        gateway!.IpAddress,
                        notification.BinId
                        );
        }
    }

}
