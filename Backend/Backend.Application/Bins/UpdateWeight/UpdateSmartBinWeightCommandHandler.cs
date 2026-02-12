using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Bins;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Bins.UpdateWeight
{
    internal class UpdateSmartBinWeightCommandHandler(IApplicationDbContext _dbContext,
        ILogger<UpdateSmartBinWeightCommandHandler> _logger) : ICommandHandler<UpdateBinWeightCommand>
    {
        public async Task<Result> Handle(UpdateBinWeightCommand command, CancellationToken cancellationToken)
        {
            var bin = await _dbContext.Bins
                .Include(s=> s.InventoryItem)
                .SingleOrDefaultAsync(s => s.MacAddress == command.MacAddress);

            if (bin is null)
            {
                return Result.Failure(BinErrors.NotFound(command.MacAddress));
            }
            var result = bin.UpdateWeight(command.CurrentWeight);

            return await result.MatchAsync(
                onSuccess: async () =>
                {
                    _dbContext.Bins.Update(bin);
                    await _dbContext.SaveChangesAsync();
                    return result;
                },
                onFailure: (error) =>
                {
                    _logger.LogError("Weight update can not be processed, because the bin with id: {BinId} is not configured yet.", bin.Id);
                    return Task.FromResult(Result.Failure(error));
                });
        }
    }
}
