using Backend.Application.Gateways.Register;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Intergration.Tests.Gateways
{
    public class GatewayTests : BaseIntegrationTest
    {
        public GatewayTests(IntegrationTestWebAppFactory factory) : base(factory)
        {
        }

        [Fact]
        public async Task Register_WithValidIpAndMacAddress_ShouldAddTheDevice()
        {
            //Arrange
            var macAddress = "00-80-41-ae-fd-7e";
            var ipAddress = "192.168.1.254";
            var command = new RegisterGatewayCommand(macAddress, ipAddress);

            //Act
            var result = await Sender.Send(command);

            //Assert
            var device = await DbContext.Gateways.SingleOrDefaultAsync(d => d.MacAddress == macAddress);
            Assert.True(result.IsSuccess);
            Assert.NotNull(device);
        }

        [Fact]
        public async Task Register_WithInvlaidValidIpAddress_ShouldNotAddTheDevice()
        {
            //Arrange
            var ipAdress = "192.168.1";
            var macAddress = "00-80-41-ae-fd-7e";
            var command = new RegisterGatewayCommand(macAddress, ipAdress);

            //Act
            var result = await Sender.Send(command);

            //Assert
            var device = await DbContext.Gateways.FirstOrDefaultAsync(d => d.IpAddress == ipAdress);
            Assert.True(result.IsFailure);
            Assert.Null(device);
        }

        [Fact]
        public async Task Register_WithInvlaidValidMacAddress_ShouldNotAddTheDevice()
        {
            //Arrange
            var ipAdress = "192.168.12";
            var macAddress = "FF";
            var command = new RegisterGatewayCommand(macAddress, ipAdress);

            //Act
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsFailure);
            var device = await DbContext.Gateways.SingleOrDefaultAsync(d => d.MacAddress == macAddress);
            Assert.Null(device);
        }

        [Fact]
        public async Task Register_UpdateExistingDevice_ShouldUpdateTheIpAddress()
        {
            //Arrange
            var registerCommand = new RegisterGatewayCommand("00-80-41-af-fd-7d", "192.168.1.123");
            var updateCommand = new RegisterGatewayCommand("00-80-41-af-fd-7d", "192.168.1.122");
            //Act
            await Sender.Send(registerCommand);
            var result = await Sender.Send(updateCommand);
            //Assert
            var device = await DbContext.Gateways.SingleOrDefaultAsync(d => d.IpAddress == updateCommand.IpAddress);
            Assert.True(result.IsSuccess);
            Assert.NotNull(device);
            Assert.Equal(updateCommand.IpAddress, device.IpAddress);
        }

        [Fact]
        public async Task Register_DuplicatedIPAndMacAddresss_ShouldFail()
        {
            //Arrange
            var registerCommand = new RegisterGatewayCommand("00-80-41-af-fd-7d", "192.168.1.123");
            var updateCommand = new RegisterGatewayCommand("00-80-41-af-fd-7d", "192.168.1.123");
            //Act
            var resgisterResult = await Sender.Send(registerCommand);
            var updateResult = await Sender.Send(updateCommand);
            //Assert
            var device = await DbContext.Gateways.SingleOrDefaultAsync(d => d.MacAddress == registerCommand.MacAddress);
            Assert.True(updateResult.IsFailure);
            Assert.NotNull(device);
            Assert.Equal(registerCommand.IpAddress, device.IpAddress);
        }

    }
}
