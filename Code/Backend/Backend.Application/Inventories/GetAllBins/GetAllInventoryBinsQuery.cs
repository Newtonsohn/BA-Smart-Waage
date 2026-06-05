using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;

namespace Backend.Application.Inventories.GetAllBins
{
    public record GetAllInventoryBinsQuery(): IQuery<IReadOnlyList<BinInventoryResponse>>;
}
