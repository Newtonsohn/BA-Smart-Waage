using Backend.Application.Bins.AssignInventoryItem;
using Backend.Application.Bins.Register;
using Backend.Application.Bins.UpdateWeight;
using Backend.Application.Gateways.Register;
using Backend.Application.Inventories.Create;
using Backend.Domain.Inventories;
using Backend.Domain.Kernel;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Intergration.Tests.SmartBins
{
    public class UpdateSmartBinWeightTests : BaseIntegrationTest
    {
        public UpdateSmartBinWeightTests(IntegrationTestWebAppFactory factory) : base(factory)
        {
        }

        [Fact]
        public async Task UpdateWeight_WithUnconfiguredSmartBin_WeightUpdateWasNotStored()
        {
            //Arrange
            var macAddress = "FF:FF:FF:FF:FF:FF";
            var smartBin = await CreateSmartBin("SmartBin 1", macAddress);

            //Act
            var command = new UpdateBinWeightCommand(15.64f, macAddress);
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsFailure);
            var weightUpdates = await DbContext.BinWeightUpdates.Where(u => u.BinId == smartBin).ToListAsync();
            Assert.Empty(weightUpdates);
        }

        [Fact]
        public async Task UpdateWeight_WithInvalidMacAddress_WeightUpdateWasStored()
        {
            //Arrange
            var macAddress = "FF:FF:FF:FF:FF:EE";
            var smartBin = await CreateSmartBin("SmartBin 2", macAddress);

            //Act
            var command = new UpdateBinWeightCommand(15.64f, "FF:FF:FF:FF:FF:EF");
            var result = await Sender.Send(command);

            //Assert
            Assert.True(result.IsFailure);
            Assert.Equal(ErrorType.NotFound, result.Error.Type);
        }

        [Fact]
        public async Task SendWeigtUdpates_WithValidWeightUpdates_AllUpdatesWasStored()
        {
            //Arrange
            var macAddress = "FF:FF:FF:FF:FF:DD";
            var bin = await CreateSmartBin("Bin 1", macAddress);
            var random = new Random();

            var createInventoryItemCommand = new CreateInventoryItemCommand("TestItem 1", "FFFF.FFFF", 10, 10, StockIndicator.Quantity);
            var result = await Sender.Send(createInventoryItemCommand);


            var assignItemCommand = new AssignInventoryItemCommand(bin, result.Value, 5, 40);
            await Sender.Send(assignItemCommand);

           
            //Act
            for (var i = 0; i < 10; i++)
            {
                float noise = (float)((random.NextDouble() - 0.5) * 1.0);
                float noisyWeight = ((i + 1) * 5.0f) + noise;
                var command = new UpdateBinWeightCommand((i + 1) * 5.0f, macAddress);
                await Sender.Send(command);
            }
            await Task.Delay(6000);

            //Assert
            var entriesCount = DbContext.BinWeightUpdates.Where(x => x.BinId == bin).Count();
            Assert.Equal(10, entriesCount);
        }

        private async Task<Guid> CreateSmartBin(string Name, string MacAddress)
        {
            var registerCommand = new RegisterGatewayCommand("00-80-41-ae-fd-7e", "192.168.1.254");
            await Sender.Send(registerCommand);

            var gateway = await DbContext.Gateways.FirstOrDefaultAsync(g => g.MacAddress == "00-80-41-ae-fd-7e");
            var command = new RegisterBinCommand(Name, MacAddress, gateway!.Id);
            var result = await Sender.Send(command);
            return result.Value;
        }
    }
}
