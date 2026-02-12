
using Backend.Application.Gateways.Register;
using EdgeDevice.Network;

namespace EdgeDevice.Register
{
    public class RegisterGatewayOnStartupService(IHttpClientFactory _httpClientFactory, INetworkInformationProvider _networkInformationProvider, ILogger<RegisterGatewayOnStartupService> _logger) : IHostedService
    {
        public async  Task  StartAsync(CancellationToken cancellationToken)
        {
            await RegisterGatewayOnBackend();
        }

        public Task StopAsync(CancellationToken cancellationToken)
        {
            return Task.CompletedTask;
        }

        private async Task RegisterGatewayOnBackend()
        {
            using var client = _httpClientFactory.CreateClient("BackendClient");
            var command = new RegisterGatewayCommand(_networkInformationProvider.GetMacAddress().Value, _networkInformationProvider.GetIpAddress().Value);
            var response = await client.PutAsJsonAsync("/gateways/register", command);
            _logger.LogWarning("Registered Gatway on backend with statuscode: {StatusCode}", response.StatusCode);
            //response.EnsureSuccessStatusCode();
        }
    }


}
