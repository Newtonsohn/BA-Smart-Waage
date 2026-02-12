using Backend.Domain.Bins;

namespace Backend.Application.Gateways.Shared
{
    public record BinResponse(Guid Id, string Name, string MacAddress)
    {
        public BinResponse(Bin bin) : this(bin.Id, bin.DeviceName, bin.MacAddress) { }
    }

   
}
