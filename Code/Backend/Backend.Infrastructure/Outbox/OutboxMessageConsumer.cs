namespace Backend.Infrastructure.Outbox;
public class OutboxMessageConsumer
{
    public Guid Id { get; set; }
    public string ConsumerName { get; set; } = string.Empty;
}
