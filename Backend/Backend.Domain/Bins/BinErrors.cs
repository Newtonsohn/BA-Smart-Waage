using Backend.Domain.Kernel;

namespace Backend.Domain.Bins
{
    public static class BinErrors
    {
        public static Error AlreadyRegistered(string deviceName) => Error.Failure(
             "BinDevice.AlreadyAssigned",
             $"The bin with macaddress = '{deviceName}' is already registred.");

        public static Error AlreadyAssigned(Guid id) => Error.Conflict(
             "BinDevice.AlreadyAssigned",
             $"The bin with id = '{id}' is already assigned to this gateway.");
        public static Error NotFound(Guid id) => Error.NotFound(
          "BinDevice.AlreadyAssigned",
          $"The bin with id = '{id}' could not be found.");

        public static Error NotFound(string macAddress) => Error.NotFound(
         "BinDevice.AlreadyAssigned",
         $"The bin with mac address = {macAddress} could not be found.");

        public static Error NotConfigured(Guid id) => Error.Problem(
             "BinDevice.NotConfigured",
             $"The bin with id = '{id}' is not configured."
            );
    }
}
