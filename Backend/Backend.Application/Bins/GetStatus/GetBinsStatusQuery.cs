using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.GetStatus
{
    public record GetBinsStatusQuery: IQuery<IReadOnlyList<BinStatusResponse>>;
}
