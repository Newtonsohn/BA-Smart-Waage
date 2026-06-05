namespace Backend.Domain.Bins
{
    public class BinWeightUpdate
    {
        public Guid Id { get; init; } = Guid.CreateVersion7();
        /// <summary>
        /// Weight in grams
        /// </summary>
        public double Weight { get; init; }
        /// <summary>
        /// TimeStamp in UTC.
        /// </summary>
        public DateTime TimeStamp { get; init; }
        public Guid BinId { get; init; }
    }
}
