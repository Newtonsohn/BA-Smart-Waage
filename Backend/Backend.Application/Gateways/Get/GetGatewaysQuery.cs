using Backend.Application.Abstractions.Messaging;
using Backend.Application.Gateways.Shared;

namespace Backend.Application.Gateways.Get
{
    public record GetGatewaysQuery(): IQuery<List<GatewayResponse>>;
}
