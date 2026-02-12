using Backend.Domain.Kernel;

namespace Backend.Domain.Bins.Events
{
    public record BinDeviceRegisteredDomainEvent(Guid BinId, Guid GatewayId, string DeviceName, string MacAddress): DomainEvent;

}
