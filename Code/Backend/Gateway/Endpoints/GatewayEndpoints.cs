using EdgeDevice.BLE;
using Microsoft.AspNetCore.Mvc;

namespace EdgeDevice.Endpoints
{
    public static class GatewayEndpoints
    {
        public static void MapEdgeDevice(this IEndpointRouteBuilder endpointRouteBuilder)
        {
            RouteGroupBuilder endpoints = endpointRouteBuilder.MapGroup("bins");
            endpoints.MapPost("/update", OnUpdateSmartBins);
        }

        private async static Task<IResult> OnUpdateSmartBins([FromBody] ISet<string> macAddresses,[FromServices] IBLEService service)
        {
            var result = await service.UpdatedAssignedDevicesAsync(macAddresses);
            if (result.IsSuccess)
            {
                return Results.Ok();
            }
            else
            {
                return Results.Problem();
            }
        }
    }
}
