using Backend.Application.Abstractions.Data;
using Backend.Application.Abstractions.Messaging;
using Backend.Domain.Gateways;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Gateways.Register
{
    internal class RegisterGatewayCommandHandler(IApplicationDbContext _dbContext) : ICommandHandler<RegisterGatewayCommand, bool>
    {
        public async Task<Result<bool>> Handle(RegisterGatewayCommand command, CancellationToken cancellationToken)
        {
            if (await _dbContext.Gateways.AnyAsync(d => d.IpAddress == command.IpAddress && d.MacAddress == command.MacAddress))
            {
                return Result.Failure<bool>(GatewayErrors.AlreadyRegistered(command.IpAddress));
            }

            var gw = await _dbContext.Gateways.FirstOrDefaultAsync(d => d.MacAddress == command.MacAddress);
            if (gw is null)
            {
                gw = Gateway.Create(Guid.NewGuid(), command.IpAddress, command.MacAddress);
                _dbContext.Gateways.Add(gw);
                await _dbContext.SaveChangesAsync(cancellationToken);
                return Result.Success(true);
            }
            else
            {
                gw.IpAddress = command.IpAddress;
                await _dbContext.SaveChangesAsync(cancellationToken);
                return Result.Success(false);
            }


        }
    }
}
