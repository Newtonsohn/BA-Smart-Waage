using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;

namespace Backend.Application.Inventories.GetInventoryItemById
{
    public record GetInventoryItemByIdQuery(Guid Id): IQuery<InventoryItemResponse>;
}
