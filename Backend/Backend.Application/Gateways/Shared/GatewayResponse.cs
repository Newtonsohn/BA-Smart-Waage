using Backend.Domain.Gateways;

namespace Backend.Application.Gateways.Shared
{
    public record GatewayResponse(Guid Id, string IpAddress, string MacAddress, List<BinResponse> AssignedBins)
    {
        public GatewayResponse(Gateway gateway)
            : this(
                  gateway.Id,
                  gateway.IpAddress,
                  gateway.MacAddress,
                  gateway.Bins.Select(s => new BinResponse(s)).ToList()
                  )
        { }
    }

   
}
