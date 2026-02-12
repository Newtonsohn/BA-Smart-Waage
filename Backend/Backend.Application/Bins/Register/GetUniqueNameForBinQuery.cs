using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.Register
{
    public record GetUniqueNameForBinQuery(): IQuery<string>;
}
