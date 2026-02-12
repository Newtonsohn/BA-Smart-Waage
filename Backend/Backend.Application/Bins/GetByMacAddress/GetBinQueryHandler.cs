using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Bins;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.GetByMacAddress
{
    internal class GetBinQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetBinQuery, BinResponse>
    {
        public async Task<Result<BinResponse>> Handle(GetBinQuery query, CancellationToken cancellationToken)
        {
            var bin = await _dbContext.Bins.FirstOrDefaultAsync(s => s.MacAddress == query.MacAddress);
            if(bin != null) 
            {
                var configuration = new BinResponse(bin);
                return Result.Success(configuration);
            }
            return Result.Failure<BinResponse>(BinErrors.NotFound(query.MacAddress));
        }
    }
}
