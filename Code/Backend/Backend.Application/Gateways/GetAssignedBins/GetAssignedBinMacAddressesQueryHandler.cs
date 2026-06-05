using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Gateways;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;

namespace Backend.Application.Gateways.GetAssignedBins
{
    internal class GetAssignedBinMacAddressesQueryHandler(IApplicationDbContext _dbContext, ILogger<GetAssignedBinMacAddressesQueryHandler> _logger) : IQueryHandler<GetAssignedBinMacAddressesQuery, ISet<string>>
    {
        public async Task<Result<ISet<string>>> Handle(GetAssignedBinMacAddressesQuery query, CancellationToken cancellationToken)
        {
            if(!await _dbContext.Gateways.AnyAsync(x => x.MacAddress == query.GatewayMacAddress))
            {
                _logger.LogError("Gatway with MAC address: {MacAddress} could not be found.", query.GatewayMacAddress);
                return Result.Failure<ISet<string>>(GatewayErrors.NotFound(query.GatewayMacAddress));
            }

            ISet<string> binMacAddresses = _dbContext.Gateways.SelectMany(x => x.Bins.Select(s => s.MacAddress)).ToHashSet();
            return Result.Success(binMacAddresses);
        }
    }
}
