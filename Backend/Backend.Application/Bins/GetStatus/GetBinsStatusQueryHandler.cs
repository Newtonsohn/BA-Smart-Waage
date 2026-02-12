using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Inventories.Contracts;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Bins.GetStatus
{
    internal class GetBinsStatusQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetBinsStatusQuery, IReadOnlyList<BinStatusResponse>>
    {
        public async Task<Result<IReadOnlyList<BinStatusResponse>>> Handle(GetBinsStatusQuery request, CancellationToken cancellationToken)
        {
            var bins = await _dbContext.Bins.ToListAsync(cancellationToken);

            var binResponses = new List<BinStatusResponse>();
            foreach (var bin in bins)
            {
                binResponses.Add(new BinStatusResponse(bin.Id, bin.DeviceName, bin.MacAddress, bin.IsOnline, bin.LastSeen, new BinInventoryResponse(bin)));
            }

            return Result.Success(binResponses as IReadOnlyList<BinStatusResponse>);            
        }
    }
}
