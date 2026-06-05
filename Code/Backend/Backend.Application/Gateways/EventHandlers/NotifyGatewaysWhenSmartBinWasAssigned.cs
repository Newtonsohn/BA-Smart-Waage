using Backend.Application.Abstractions.Data;
using MediatR;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Backend.Domain.Bins.Events;
using Microsoft.Extensions.Configuration;

namespace Backend.Application.Gateways.EventHandlers
{
    internal class NotifyGatewaysWhenSmartBinWasAssigned(
        IHttpClientFactory _httpClientFactory,
        ILogger<NotifyGatewaysWhenSmartBinWasAssigned> _logger,
        IServiceScopeFactory _serviceScopeFactory) : NotifiyGatewayDomainEventHandler, INotificationHandler<BinAssignedDomainEvent>
    {
        public async Task Handle(BinAssignedDomainEvent notification, CancellationToken cancellationToken)
        {
            using var scope = _serviceScopeFactory.CreateScope();
            var dbContext = scope.ServiceProvider.GetRequiredService<IApplicationDbContext>();
            var configuration = scope.ServiceProvider.GetRequiredService<IConfiguration>();
            var oldGateway = await dbContext.Gateways.Include(g => g.Bins).FirstOrDefaultAsync(g => g.Id == notification.OldGatewayId, cancellationToken);
            var newGateway = await dbContext.Gateways.Include(g => g.Bins).FirstOrDefaultAsync(g => g.Id == notification.NewGatewayId, cancellationToken);

            var client = _httpClientFactory.CreateClient("GatewayClient");

            var oldGatewayRequestUrl = GetGatewayUpdateUrl(oldGateway!.IpAddress, configuration);
            var newGatewayRequestUrl = GetGatewayUpdateUrl(newGateway!.IpAddress, configuration);

            await UpdateGateway(oldGateway, client, oldGatewayRequestUrl);
            await UpdateGateway(newGateway, client, newGatewayRequestUrl);

            _logger.LogInformation(
                "Affected Gateways [{OldGateway},{NewGateway}] were updated for bin: {Bin}", 
                oldGateway!.IpAddress,
                newGateway!.IpAddress,
                notification.BinId
                );
        }

       
    }
}
