using Backend.Domain.Bins;
using Backend.Domain.Bins.Events;
using Backend.Domain.Gateways.Events;
using Backend.Domain.Kernel;


namespace Backend.Domain.Gateways
{
    public class Gateway : Aggregate
    {
        private List<Bin> _bins = new List<Bin>();

        public required string MacAddress { get; init; }
        public required string IpAddress { get; set; }
        public IReadOnlyCollection<Bin> Bins => _bins;

        public Bin RegisterBin(string deviceName, string macAddress)
        {
            var bin = new Bin()
            {
                DeviceName = deviceName,
                MacAddress = macAddress,
                LastSeen = DateTime.UtcNow,
                GatewayId = Id
            };
            _bins.Add(bin);
            Raise(new BinDeviceRegisteredDomainEvent(bin.Id, Id, bin.DeviceName, bin.MacAddress));
            return bin;
        }
        public void ReleaseSmartBin(Bin device)
        {
            _bins.Remove(device);
        }

        public Result AssignBin(Bin device)
        {
            if (_bins.Any(bin => bin.Id == device.Id))
            {
                return Result.Failure(BinErrors.AlreadyAssigned(device.Id));
            }
            var oldGatewayId = device.GatewayId;
            device.GatewayId = Id;
            _bins.Add(device);
            Raise(new BinAssignedDomainEvent(device.Id, Id, oldGatewayId));
            return Result.Success();
        }

        public static Gateway Create(Guid id, string ipAddress, string macAddress)
        {
            var gw = new Gateway()
            {   IpAddress = ipAddress,
                MacAddress = macAddress
            };

            gw.Raise(new GatewayRegisteredDomainEvent(gw.Id, gw.MacAddress, gw.IpAddress));
            return gw;
        }
    }
}
