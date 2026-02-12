using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Gateways.Shared;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Gateways.Get
{
    internal class GetGatewaysQueryHandler(IApplicationDbContext _dbContext, ILogger<GetGatewaysQueryHandler> _logger) : IQueryHandler<GetGatewaysQuery, List<GatewayResponse>>
    {
        public async Task<Result<List<GatewayResponse>>> Handle(GetGatewaysQuery request, CancellationToken cancellationToken)
        {
            try
            {
                return Result.Success(await _dbContext.Gateways.Include(g => g.Bins).Select(g => new GatewayResponse(g)).ToListAsync(cancellationToken));
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Unexpected error occured during GetGatewaysQuery");
                return Result.Failure<List<GatewayResponse>>(Error.UnexpectedFailure);
            }
        }
    }
}
