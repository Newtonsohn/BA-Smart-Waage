using Backend.Domain.Bins.Events;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using System.ComponentModel.DataAnnotations.Schema;

namespace Backend.Domain.Bins
{
    public class Bin : Aggregate
    {
        private const int AvailabilityLowerBoundFactor = -2;
        private const double Epsilon = 0.1;
        public string DeviceName { get; set; } = string.Empty;
        public required string MacAddress { get; init; }
        public DateTime LastSeen { get; set; }
        public Guid GatewayId { get; set; }
        /// <summary>
        /// Treshold value in number of items if InventoryItem.Indicator == Quantity else Treshold in %
        /// </summary>
        public double Treshold { get; private set; }
        /// <summary>
        /// Capacity in grams
        /// </summary>
        public double Capacity { get; set; }
        /// <summary>
        /// Update interval when the bin checks for changes in [s]
        /// </summary>
        public int UpdateInterval { get; private set; }
        /// <summary>
        /// Maximum interval in seconds between updates sent by the bin, even if there are no changes.
        /// </summary>
        public int HeartbeatInterval { get; private set; }

        public Guid? InventoryItemId { get; set; }
        public InventoryItem? InventoryItem { get; private set; }
        /// <summary>
        /// Weight in grams
        /// </summary>
        public double CurrentWeight { get; private set; }

        [NotMapped]
        public bool IsOnline { get
            {
                var lowerBoundary = DateTime.UtcNow.AddSeconds(AvailabilityLowerBoundFactor * HeartbeatInterval);
                return lowerBoundary < LastSeen;
            } }

        [NotMapped]
        public bool IsConfigured => InventoryItem is not null;

        public void HasBeenSeen()
        {
            LastSeen = DateTime.UtcNow;
        }
        
        public void AssignInventoryItem(InventoryItem item, double treshold, int capacity)
        {
            Treshold = treshold;
            Capacity = capacity * item.ItemWeight;
            var e = new InventoryItemAssignedDomainEvent(Id, item.Id, InventoryItem?.Id);
            InventoryItemId = item.Id;
            InventoryItem = item;
            Raise(e);
        }

        public void Configure(
            string name,
            int UpdateInterval,
            int HeartBeatInterval
            )
        {
            DeviceName = name;
            HeartbeatInterval = HeartBeatInterval;
            this.UpdateInterval = UpdateInterval;
        }
            
        public Result UpdateWeight(double newWeight)
        {
            if(!IsConfigured)
            {
                return Result.Failure(BinErrors.NotConfigured(Id));
            }

            LastSeen = DateTime.UtcNow;
            var oldItemCount = ItemCount;
            var oldWeight = CurrentWeight;

            if (Math.Abs(oldWeight - newWeight) < Epsilon)
            {
                return Result.Success();
            }

            newWeight = newWeight < 0 ? 0 : newWeight;

            CurrentWeight = newWeight;
            Raise(new BinWeightChangedDomainEvent(Id,
                DeviceName, 
                InventoryItem!.Id, 
                newWeight,
                oldWeight,
                InventoryItem!.ItemWeight,
                ItemCount,
                oldItemCount,
                LastSeen));

            return Result.Success();
        }

        [NotMapped]
        public int ItemCount => InventoryItem is not null ? InventoryItemCount.GetCount(CurrentWeight, InventoryItem.ItemWeight) : 0;

        /// <summary>
        /// Fill level in %
        /// </summary>
        [NotMapped]
        public double FillLevel => CurrentWeight / Capacity * 100;

        /// <summary>
        /// Determines whether the bin is considered empty based on its configuration and stock indicator type.
        /// Returns <c>true</c> if the bin is not configured, or if the fill level or item count falls below the defined threshold.
        /// </summary>
        /// <returns><c>true</c> if the bin is empty; otherwise, <c>false</c>.</returns>
        public bool IsEmpty()
        {
            if (!IsConfigured)
                return true;

            return InventoryItem!.Indicator switch
            {
                StockIndicator.Percent => FillLevel < Treshold,
                StockIndicator.Quantity => ItemCount < (int)Treshold,
                _ => true
            };
        }
    }
}
