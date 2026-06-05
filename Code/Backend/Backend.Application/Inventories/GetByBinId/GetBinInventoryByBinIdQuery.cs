using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;

namespace Backend.Application.Inventories.GetByBinId
{
    public record GetBinInventoryByBinIdQuery(Guid BinId): IQuery<BinInventoryResponse>;
}
