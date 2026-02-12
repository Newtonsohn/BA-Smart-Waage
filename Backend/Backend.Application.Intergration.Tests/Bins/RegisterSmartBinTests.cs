using Backend.Application.Bins.Register;
using Backend.Application.Gateways.Register;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Intergration.Tests.SmartBins
{
    public class RegisterSmartBinTests : BaseIntegrationTest
    {
        public RegisterSmartBinTests(IntegrationTestWebAppFactory factory) : base(factory)
        {

        }

        [Fact]
        public async Task RegisterSmartBin_WithValidMacAddress_SmartBinIsAssignedToGateway()
        {
            //Arrange
            var macAddressGateway = "00-80-41-ae-fd-77";
            var ipAddress = "192.168.1.254";
            var registerGatewayCommand = new RegisterGatewayCommand(macAddressGateway, ipAddress);
            await Sender.Send(registerGatewayCommand);

            var gateway = await DbContext.Gateways.AsNoTracking().FirstOrDefaultAsync(g => g.MacAddress == macAddressGateway);
            Assert.NotNull(gateway);

            //Act
            var macAddress = "00-80-41-ae-fd-7e";
            var command = new RegisterBinCommand("SmartBin-1", macAddress, gateway.Id);
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsSuccess);
            var smartBin = await DbContext.Bins.FirstOrDefaultAsync(s => s.Id == result.Value);
            Assert.NotNull(smartBin);

            var assignedSmartBinIds = await DbContext.Gateways
            .Where(g => g.Id == gateway.Id)
            .SelectMany(g => g.Bins.Select(s => s.Id)).ToListAsync();

            Assert.Single(assignedSmartBinIds);
            Assert.Equal(smartBin.Id, assignedSmartBinIds.First());
        }

        [Fact]
        public async Task RegisterSmartBin_WithInvalidMacAddress_SmartBinIsNotRegistered()
        {
            //Act
            var macAddress = "00-FF";
            var command = new RegisterBinCommand("SmartBin-1", macAddress, Guid.NewGuid());
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsFailure);
            var smartBin = await DbContext.Bins.FirstOrDefaultAsync(s => s.MacAddress == macAddress);
            Assert.Null(smartBin);
        }

        [Fact]
        public async Task RegisterSmartBin_WhenAlreadyRegistered_ResultIsFailure()
        {
            //Arrange
            var macAddressGateway = "00-80-41-ae-fd-77";
            var ipAddress = "192.168.1.254";
            var registerGatewayCommand = new RegisterGatewayCommand(macAddressGateway, ipAddress);
            await Sender.Send(registerGatewayCommand);

            var gateway = await DbContext.Gateways.AsNoTracking().FirstOrDefaultAsync(g => g.MacAddress == macAddressGateway);
            Assert.NotNull(gateway);

            //Act
            var macAddress = "00-80-41-ae-fd-11";
            var firstCommand = new RegisterBinCommand("SmartBin-1", macAddress, gateway.Id);
            var secondCommand = new RegisterBinCommand("SmartBin-2", macAddress, gateway.Id);
            var firstResult = await Sender.Send(firstCommand);
            var secondResult = await Sender.Send(secondCommand);

            //Assert
            Assert.True(firstResult.IsSuccess);
            Assert.True(secondResult.IsFailure);

            var smartBin = await DbContext.Bins.SingleAsync(s => s.Id == firstResult.Value);
            Assert.NotNull(smartBin);
            Assert.Equal(firstCommand.DeviceName, smartBin.DeviceName);
            
        }

    }
}
