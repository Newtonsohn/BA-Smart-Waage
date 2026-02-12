using Backend.Application.Abstractions.Messaging;

namespace Backend.Application.Bins.UpdateWeight
{
    /// <summary>
    /// Command to update the weight from a bin.
    /// </summary>
    /// <param name="CurrentWeight">Weight in grams</param>
    /// <param name="MacAddress"></param>
    public record UpdateBinWeightCommand(float CurrentWeight, string MacAddress): ICommand;
}
