using MediatR;

namespace Backend.Domain.Kernel;

public abstract record DomainEvent() : IDomainEvent
{
    public Guid Id { get; init; } = Guid.NewGuid();
}
public interface IDomainEvent : INotification
{
    public Guid Id { get; }
}
