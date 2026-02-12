using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.Register
{
    internal class GetUniqueNameForBinQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetUniqueNameForBinQuery, string>
    {
        public async Task<Result<string>> Handle(GetUniqueNameForBinQuery query, CancellationToken cancellationToken)
        {
            var binCount = await _dbContext.Bins.CountAsync(cancellationToken);
            return $"Bin {++binCount}";
        }
    }
}
