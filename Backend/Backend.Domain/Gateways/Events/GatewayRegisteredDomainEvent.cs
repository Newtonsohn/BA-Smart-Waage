using Backend.Domain.Kernel;

namespace Backend.Domain.Gateways.Events
{
    public record GatewayRegisteredDomainEvent(Guid Id, string MacAddress, string IpAddress) : IDomainEvent;
}
