using Backend.Domain.Gateways;
using Microsoft.Extensions.Configuration;
using System.Text;
using System.Text.Json;

namespace Backend.Application.Gateways.EventHandlers
{
    internal abstract class NotifiyGatewayDomainEventHandler
    {
        protected static async Task UpdateGateway(Gateway gateway, HttpClient client, Uri requestUri)
        {
            var smartBinIds = gateway.Bins.Select(sb => sb.MacAddress).ToHashSet();
            var jsonPayload = JsonSerializer.Serialize(smartBinIds);
            var content = new StringContent(jsonPayload, Encoding.UTF8, "application/json");
            var response = await client.PostAsync(requestUri, content);
            response.EnsureSuccessStatusCode();
        }
        protected static Uri GetGatewayUpdateUrl(string ipAddress, IConfiguration configuration)
        {
            var port = configuration.GetValue<string>("GatewayPort");
            return new Uri($"http://{ipAddress}:{port}/bins/update");
        }
    }
}
