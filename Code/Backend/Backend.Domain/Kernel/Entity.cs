namespace Backend.Domain.Kernel;

public abstract class Entity
{
    public Guid Id { get; init; } = Guid.NewGuid();
}
