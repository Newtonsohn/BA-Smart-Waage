using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Inventories.GetByBinId
{
    internal class GetBinInventoryByBinIdQueryHandler(
        IApplicationDbContext _dbContext,
        ILogger<GetBinInventoryByBinIdQueryHandler> _logger
        ) : IQueryHandler<GetBinInventoryByBinIdQuery, BinInventoryResponse>
    {
        public async Task<Result<BinInventoryResponse>> Handle(GetBinInventoryByBinIdQuery query, CancellationToken cancellationToken)
        {
            var bin = await _dbContext.Bins.FirstOrDefaultAsync(s => s.Id == query.BinId);
            if (bin is null)
            {
                _logger.LogError("There is no bin registered for id: {Id}", query.BinId);
                return Result.Failure<BinInventoryResponse>(InventoryErrors.BinNotFound(query.BinId));
            }
            
            if(!bin.IsConfigured)
            {
                return Result.Failure<BinInventoryResponse>(InventoryErrors.BinNotConfigured);
            }
            
           return Result.Success(new BinInventoryResponse(bin));
        }
    }
}
