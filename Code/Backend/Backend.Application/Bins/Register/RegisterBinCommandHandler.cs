using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Bins;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.Register
{
    internal class RegisterBinCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<RegisterBinCommand, Guid>
    {
        public async Task<Result<Guid>> Handle(RegisterBinCommand command, CancellationToken cancellationToken)
        {
            if (await _dbContext.Bins.AnyAsync(s => s.MacAddress == command.MacAddress))
            {
                return Result.Failure<Guid>(BinErrors.AlreadyRegistered(command.MacAddress));
            }

            var gateway = await _dbContext.Gateways
                .Include(g => g.Bins)
                .FirstOrDefaultAsync(g => g.Id == command.GatewayIdToAssign);

            ArgumentNullException.ThrowIfNull(gateway, nameof(gateway));

            var bin = gateway.RegisterBin(command.DeviceName, command.MacAddress);

            _dbContext.Bins.Add(bin);
            await _dbContext.SaveChangesAsync(cancellationToken);
            return Result.Success(bin.Id);
        }
    }
}
