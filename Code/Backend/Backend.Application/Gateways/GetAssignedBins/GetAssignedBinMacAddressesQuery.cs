using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Gateways.GetAssignedBins
{
    public record GetAssignedBinMacAddressesQuery(string GatewayMacAddress): IQuery<ISet<string>>;
}
