using Backend.Application.Bins.Assign;
using Backend.Application.Bins.Register;
using Backend.Application.Gateways.Register;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Intergration.Tests.Bins
{
    public class AssignBinTests : BaseIntegrationTest
    {
        public AssignBinTests(IntegrationTestWebAppFactory factory) : base(factory)
        {
        }

        [Fact]
        public async Task AssignBin_WithIds_BinShouldBeAssingedToGateway()
        {
            //Arrange
            var firstGatewayId = await RegisterGateway("00-80-22-ae-fd-78", "192.168.11.111");
            var secondGatewayId = await RegisterGateway("00-80-23-ae-fd-78", "192.168.21.111");
            var firstBinId = await RegisterBins("Bin-1", "00-80-41-ae-fd-78", firstGatewayId);
            var secondBinId = await RegisterBins("Bin-2", "00-80-41-ae-fd-79", firstGatewayId);

            //Act
            var command = new AssignBinCommand(secondBinId, secondGatewayId);
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsSuccess);

            var firstGateway = await DbContext.Gateways
               .Include(g => g.Bins)
               .FirstOrDefaultAsync(g => g.Id == firstGatewayId);
            Assert.NotNull(firstGateway);

            var secondGateway = await DbContext.Gateways
                .Include( g => g.Bins)
                .FirstOrDefaultAsync(g => g.Id == secondGatewayId);
            Assert.NotNull(secondGateway);

            Assert.Single(firstGateway.Bins);
            Assert.Single(secondGateway.Bins);
            Assert.Equal(firstBinId, firstGateway.Bins.First().Id);
            Assert.Equal(secondBinId, secondGateway.Bins.First().Id);
        }

        [Fact]
        public async Task AssignBin_WhenAlreadyAssigned_ResultIsFailure()
        {
            //Arrange
            var macAddressGateway = "00-80-41-ae-fd-11";
            var ipAddress = "192.168.1.111";
            var gatewayId = await RegisterGateway(macAddressGateway, ipAddress);
            var binId = await RegisterBins("Bin-1", "00-80-41-ae-fd-22", gatewayId);

            //Act
            var command = new AssignBinCommand(binId, gatewayId);
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsFailure);

            var gateway = await DbContext.Gateways
               .Include(g => g.Bins)
               .FirstOrDefaultAsync(g => g.Id == gatewayId);
            Assert.NotNull(gateway);

            Assert.Single(gateway.Bins);
            Assert.Equal(binId, gateway.Bins.First().Id);
        }

        private async Task<Guid> RegisterBins(string deviceName, string macAddress, Guid gatewayId)
        {
            var command = new RegisterBinCommand(deviceName, macAddress, gatewayId);
            var result = await Sender.Send(command);
            return result.Value;
        }
        private async Task<Guid> RegisterGateway(string macAddress, string ipAddress)
        {
            var registerGatewayCommand = new RegisterGatewayCommand(macAddress, ipAddress);
            await Sender.Send(registerGatewayCommand);
            return (await DbContext.Gateways
                .FirstOrDefaultAsync(g => g.MacAddress == macAddress))!.Id;
        }
    }
}
