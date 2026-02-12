namespace Backend.Domain.Kernel;

public interface IDateTimeProvider
{
    public DateTime UtcNow { get; }
}
