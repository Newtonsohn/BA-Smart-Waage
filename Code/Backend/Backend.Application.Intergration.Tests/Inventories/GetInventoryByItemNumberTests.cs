using Backend.Application.Bins.AssignInventoryItem;
using Backend.Application.Bins.Configure;
using Backend.Application.Bins.Register;
using Backend.Application.Bins.UpdateWeight;
using Backend.Application.Gateways.Register;
using Backend.Application.Inventories.Create;
using Backend.Application.Inventories.GetByItemNumber;
using Backend.Domain.Inventories;
using Microsoft.EntityFrameworkCore;

namespace Backend.Application.Intergration.Tests.Inventories
{
    public class GetInventoryByItemNumberTests : BaseIntegrationTest
    {
        public GetInventoryByItemNumberTests(IntegrationTestWebAppFactory factory) : base(factory)
        {
        }

        [Fact]
        public async Task GetInventoryByItemNumber_WithoutDetails_ReturnsCurrentInventoryWithoutDetails()
        {
            // Arrange
            var binMacAddress = "AA:BB:CC:DD:EE:EE";
            var itemNumber = "ITEM-125";

            var bin = await SetupBinAsync(
                    gatewayMacAddress: "AA:BB:CC:DD:EE:FF",
                    binMacAddress: binMacAddress,
                    "Bin-1.0",
                    ipAddress: "192.168.1.2");

            var itemId = await CreateInventoryItem("Testitem1", itemNumber, 2.5, 8, StockIndicator.Quantity);
            await AssignInventoryItem(bin.BinId, itemId);

            var updatedWeight = 5.05f; 
            var updateWeightCommand = new UpdateBinWeightCommand(updatedWeight, binMacAddress);
            await Sender.Send(updateWeightCommand);

            // Act
            var query = new GetInventoryByItemNumberQuery(itemNumber, ShowDetails: false);
            var result = await Sender.Send(query);
            var response = result.Value;

            // Assert
            Assert.True(result.IsSuccess);
            Assert.NotNull(response);
            Assert.Equal(itemNumber, response.ItemNumber);
            Assert.Equal(updatedWeight, response.TotalWeight);
            Assert.Equal(2, response.ItemCount);
            Assert.Empty(response.Bins);
        }

        [Fact]
        public async Task GetInventoryByItemNumber_WithDetails_ReturnsCurrentInventoryWithDetails()
        {
            // Arrange
            var firstBinMacAddress = "AA:BB:CC:DD:EE:AA";
            var secondBinMacAddress = "AA:BB:CC:DD:EE:DD";
            var itemNumber = "ITEM-123";

            (var firstBinId, var gatewayId) = await SetupBinAsync(
                    gatewayMacAddress: "AA:BB:CC:DD:AA:FF",
                    binMacAddress: firstBinMacAddress,
                    "Bin-1.2",
                    ipAddress: "192.168.1.1");

            var secondSmartBinId = await RegisterBin(secondBinMacAddress, "Bin-2.1", gatewayId);

            var itemId = await CreateInventoryItem("Testitem", itemNumber, 2.5, 8, StockIndicator.Quantity);
            await AssignInventoryItem(firstBinId, itemId);
            await AssignInventoryItem(secondSmartBinId, itemId);

            var updatedWeight = 5.05f;
            var totalWeight = updatedWeight;
            var updateWeightCommand = new UpdateBinWeightCommand(updatedWeight, firstBinMacAddress);
            await Sender.Send(updateWeightCommand);

            updatedWeight = 2.55f;
            totalWeight += updatedWeight;
            updateWeightCommand = new UpdateBinWeightCommand(updatedWeight, secondBinMacAddress);
            await Sender.Send(updateWeightCommand);

            var query = new GetInventoryByItemNumberQuery(itemNumber, ShowDetails: true);

            // Act
            var result = await Sender.Send(query);
            var response = result.Value;

            // Assert
            var e = 0.01;
            Assert.True(result.IsSuccess);
            Assert.NotNull(response);
            Assert.Equal(itemNumber, response.ItemNumber);
            Assert.InRange(response.TotalWeight, totalWeight - e, totalWeight + e);
            Assert.Equal(3, response.ItemCount);
            Assert.NotEmpty(response.Bins);
        }

        private async Task<Guid> RegisterGateway(string macAddress, string ipAddress)
        {
            var registerGatewayCommand = new RegisterGatewayCommand(macAddress, ipAddress);
            await Sender.Send(registerGatewayCommand);
            return (await DbContext.Gateways
                .FirstOrDefaultAsync(g => g.MacAddress == macAddress))!.Id;
        }

        private async Task<(Guid BinId, Guid GatewayId)> SetupBinAsync(
        string gatewayMacAddress,
        string binMacAddress,
        string deviceName,
        string ipAddress
        )
        {
            var gatewayId = await RegisterGateway(gatewayMacAddress, ipAddress);
            Guid binId = await RegisterBin(binMacAddress, deviceName, gatewayId);
            return (binId, gatewayId);
        }

        private async Task AssignInventoryItem(Guid binId, Guid inventoryItemid)
        {
            var assignItemCommand = new AssignInventoryItemCommand(binId, inventoryItemid, 5, 40);
            await Sender.Send(assignItemCommand);
        }

        private async Task<Guid> CreateInventoryItem(string itemName, string itemNumber, double itemWeight, double treshold, StockIndicator stockIndicator)
        {
            var createInventoryItemCommand = new CreateInventoryItemCommand(itemName, itemNumber, itemWeight, treshold, stockIndicator);
            var result = await Sender.Send(createInventoryItemCommand);
            return result.Value;
        }

        private async Task<Guid> RegisterBin(string binMacAddress, string deviceName, Guid gatewayId)
        {
            var registerSmartBinCommand = new RegisterBinCommand(deviceName, binMacAddress, gatewayId);
            var smartBinId = (await Sender.Send(registerSmartBinCommand)).Value;

            var configureCommand = new ConfigureBinCommand(
                BinId: smartBinId,
                DeviceName: deviceName,
                UpdateInterval: 60,
                HeartBeatInterval: 120
            );

            var result = await Sender.Send(configureCommand);
            return smartBinId;
        }
    }
}
