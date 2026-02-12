using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.GetAllBins
{
    internal class GetAllInventoryBinsQueryHandler(
        IApplicationDbContext _dbContext,
        ILogger<GetAllInventoryBinsQueryHandler> _logger) : IQueryHandler<GetAllInventoryBinsQuery, IReadOnlyList<BinInventoryResponse>>
    {
        public async Task<Result<IReadOnlyList<BinInventoryResponse>>> Handle(GetAllInventoryBinsQuery query, CancellationToken cancellationToken)
        {
            var bins = await _dbContext.Bins.ToListAsync(cancellationToken: cancellationToken);
            var configuredBins = bins.Where(b => b.IsConfigured).Select(b => new BinInventoryResponse(b)).ToList();
            return Result.Success(configuredBins as IReadOnlyList<BinInventoryResponse>);
        }
    }
}
