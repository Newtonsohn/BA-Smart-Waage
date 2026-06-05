using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Bins;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.Configure
{
    internal class ConfigureBinCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<ConfigureBinCommand>
    {
        public async Task<Result> Handle(ConfigureBinCommand command, CancellationToken cancellationToken)
        {
            var bin = await _dbContext.Bins.FirstOrDefaultAsync(s => s.Id == command.BinId);
            if (bin is null)
            {
                return Result.Failure(BinErrors.NotFound(command.BinId));
            }
            bin.Configure(command.DeviceName,
                command.UpdateInterval,
                command.HeartBeatInterval);

            _dbContext.Bins.Update(bin);
            await _dbContext.SaveChangesAsync();
            return Result.Success();
        }
    }
}
