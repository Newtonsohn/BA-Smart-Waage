using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.GetByMacAddress
{
    public record GetBinQuery(string MacAddress): IQuery<BinResponse>;
}
