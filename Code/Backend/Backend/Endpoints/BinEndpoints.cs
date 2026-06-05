using Backend.API.Extensions;
using Backend.Application.Bins.Configure;
using Backend.Application.Bins.GetByMacAddress;
using Backend.Application.Bins.UpdateWeight;
using MediatR;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace Backend.Endpoints
{
    [Authorize]
    public static class BinEndpoints
    {
        public static void MapBins(this IEndpointRouteBuilder endpointRouteBuilder)
        {
            RouteGroupBuilder endpoints = endpointRouteBuilder.MapGroup("bins").RequireAuthorization();

            endpoints.MapGet("/configuration/", OnGetConfiguration)
                .WithSummary("Get bin configuration")
                .WithDescription("Retrieves the configuration for a bin using its MAC address. Requires Gateway authorization.");

            //endpoints.MapPost("/configuration/", OnPostConfiguration)
            //    .WithSummary("Configure a bin")
            //    .WithDescription("Creates or updates a bin configuration using the provided settings. Requires authorization.");

            endpoints.MapGet("/networkConfiguration", OnGetNetworkConfiguration)
                .WithSummary("Get network configuration")
                .WithDescription("Retrieves the network configuration for a bin using its MAC address. Requires Gateway authorization.");

            endpoints.MapPut("/currentweight", OnWeightChanged)
                .WithSummary("Update bin weight")
                .WithDescription("Updates the current weight of a bin. Used by smart bins to report weight changes. Requires Gateway authorization.");
        }


        [Authorize(Policy ="Gateway")]
        [ProducesResponseType(typeof(BinResponse), StatusCodes.Status200OK)]
        private static async Task<IResult> OnGetConfiguration([FromQuery] string macAddress,
            [FromServices] ISender sender)
        {
            var query = new GetBinQuery(macAddress);
            var result = await sender.Send(query);

            if (result.IsSuccess)
            {
                return TypedResults.Ok(result.Value);
            }
            else
            {
                return result.ToProblemDetails();
            }
        }

        private static async Task<IResult> OnPostConfiguration([FromBody] ConfigureBinCommand command,
           [FromServices] ISender sender)
        {
            var result = await sender.Send(command);

            if (result.IsSuccess)
            {
                return TypedResults.Ok();
            }
            else
            {
                return result.ToProblemDetails();
            }
        }

        [ProducesResponseType(typeof(BinResponse), StatusCodes.Status200OK)]
        private static async Task<IResult> OnGetNetworkConfiguration([FromQuery] string macAddress,
           [FromServices] ISender sender)
        {
            var query = new GetBinQuery(macAddress);
            var result = await sender.Send(query);

            if (result.IsSuccess)
            {
                return TypedResults.Ok(result.Value);
            }
            else
            {
                return result.ToProblemDetails();
            }
        }

        [Authorize(Policy = "Gateway")]
        private static async Task<IResult> OnWeightChanged([FromBody] UpdateBinWeightCommand command,
         [FromServices] ISender sender)
        {
            var result = await sender.Send(command);

            if (result.IsSuccess)
            {
                return TypedResults.Ok();
            }
            else
            {
                return result.ToProblemDetails();
            }
        }
    }
}
