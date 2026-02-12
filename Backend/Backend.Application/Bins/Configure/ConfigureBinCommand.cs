using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.Configure
{
    /// <summary>
    /// Command to configure a bin.
    /// </summary>
    /// <param name="BinId"></param>
    /// <param name="DeviceName"></param>
    /// <param name="UpdateInterval">Update interval in seconds</param>
    /// <param name="HeartBeatInterval">Heatbeat interval in seconds</param>
    public record ConfigureBinCommand(
        Guid BinId,
        string DeviceName,
        int UpdateInterval,
        int HeartBeatInterval): ICommand;
}
