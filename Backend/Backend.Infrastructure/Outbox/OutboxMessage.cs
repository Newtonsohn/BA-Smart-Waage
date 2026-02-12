namespace Backend.Infrastructure.Outbox;
public sealed class OutboxMessage
{   
    public Guid Id { get; set; }
    public required string Type { get; set; }
    public required string Content { get; set; }
    public DateTime OccurredOnUtc { get; set; }
    public DateTime? ProcessedOnUtc { get; set; }

    public uint Retries { get; set; } = 0;
    public string? Error { get; set; }
}
