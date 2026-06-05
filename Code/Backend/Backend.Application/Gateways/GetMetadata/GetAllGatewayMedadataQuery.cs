using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Gateways.GetMetadata
{
    public record GatewayMetadataResponse(Guid Id, string MacAddress,string IpAddress);
    public record GetAllGatewayMedadataQuery():IQuery<List<GatewayMetadataResponse>>;
}
