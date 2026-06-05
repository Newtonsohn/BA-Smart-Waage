using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Inventories.GetInventoryHistoryByItemNumber
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="Entries"></param>
    /// <param name="From">If the query defines a 'From' parameter, that value is used; otherwise, the earliest timestamp from the returned items is used.</param>
    /// <param name="To">If the query defines a 'To' parameter, that value is used; otherwise, the latest timestamp from the returned items is used.</param>
    public record InventoryItemHistoryEntryResponse(IReadOnlyList<InventoryItemHistoryEntry> Entries, DateTime From, DateTime To);
    public record InventoryItemHistoryEntry(int PartCount, DateTime Timestamp);

    /// <summary>
    /// Query to fetch part count history of a specific smart bin. The query is limited to 5000 entries. 
    /// </summary>
    /// <param name="SmartBinId"></param>
    /// <param name="From"></param>
    /// <param name="To"></param>
    public record GetInventoryHistoryByItemNumberQuery(string ItemNumber, DateTime From, DateTime To) : IQuery<InventoryItemHistoryEntryResponse>;

}
