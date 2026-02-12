using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Application.Gateways.GetMetadata;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Gateways.Get
{
    public class GetAllGatewayMedadataQueryHandler(IApplicationDbContext _dbContext) : IQueryHandler<GetAllGatewayMedadataQuery, List<GatewayMetadataResponse>>
    {
        public async Task<Result<List<GatewayMetadataResponse>>> Handle(GetAllGatewayMedadataQuery request, CancellationToken cancellationToken)
        {
            return await _dbContext.Gateways.Select(g => new GatewayMetadataResponse(g.Id, g.MacAddress, g.IpAddress)).ToListAsync();
        }
    }
}
