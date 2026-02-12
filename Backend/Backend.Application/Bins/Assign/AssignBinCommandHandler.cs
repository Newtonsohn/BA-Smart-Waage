using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.Assign
{
    internal class AssignBinCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<AssignBinCommand>
    {
        public async Task<Result> Handle(AssignBinCommand command, CancellationToken cancellationToken)
        {
            var gateway = await _dbContext.Gateways
                .Include(g => g.Bins)
                .FirstOrDefaultAsync(g => g.Id == command.GatewayId);

            var bin = await _dbContext.Bins.FirstOrDefaultAsync(s => s.Id == command.BinId);

            ArgumentNullException.ThrowIfNull(gateway, nameof(gateway));
            ArgumentNullException.ThrowIfNull(bin, nameof(bin));
            
            var result = gateway.AssignBin(bin);
            await _dbContext.SaveChangesAsync(cancellationToken);
            return result;
        }
    }
}
