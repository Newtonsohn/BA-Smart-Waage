using Backend.Application.Inventories.Contracts;
using Backend.Application.Inventories.GetAllBins;
using Backend.Application.Inventories.GetByBinId;
using Backend.Application.Inventories.GetByItemNumber;
using Backend.Domain.Kernel;
using MediatR;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace Backend.API.Endpoints
{
    public static class InventoryEndopints
    {
        public static void MapInventory(this IEndpointRouteBuilder endpointRouteBuilder)
        {
            RouteGroupBuilder endpoints = endpointRouteBuilder.MapGroup("inventory").RequireAuthorization();

            endpoints.MapGet("/article/{itemNumber}", OnGetByItemNumber)
                .WithSummary("Get inventory by item number")
                .WithDescription("Retrieves inventory details for a specific item number. Optional 'showDetails' query flag controls detail level.");

            endpoints.MapGet("/bin/{smartBinId}", OnGetByBinId)
                .WithSummary("Get bin inventory by bin ID")
                .WithDescription("Retrieves inventory for a specific smart bin ID. The ID must be a valid GUID.");

            endpoints.MapGet("/bins", OnGetAllBins)
                .WithSummary("Get all bin summaries")
                .WithDescription("Retrieves inventory summaries for all available bins.");
        }

        [Authorize(Policy = "ExternalSystem")]
        [ProducesResponseType(typeof(InventoryResponse), StatusCodes.Status200OK)]
        public static async Task<IResult> OnGetByItemNumber(string itemNumber,
            [FromQuery]bool? showDetails,
            [FromServices] ISender sender
            )
        {
            var query = new GetInventoryByItemNumberQuery(itemNumber, showDetails ?? false);
            var result = await sender.Send(query);

            return result.Match(
                onSuccess: (response) => TypedResults.Ok(response),
                onFailure: (error) => Results.NotFound(error));
        }

        [Authorize(Policy = "ExternalSystem")]
        [ProducesResponseType(typeof(BinInventoryResponse), StatusCodes.Status200OK)]
        public static async Task<IResult> OnGetByBinId(string smartBinId, [FromServices] ISender sender)
        {
            if (Guid.TryParse(smartBinId, out var id))
            {
                var query = new GetBinInventoryByBinIdQuery(id);
                var result = await sender.Send(query);

                return result.Match(
                    onSuccess: (response) => TypedResults.Ok(response),
                    onFailure: (error) => Results.NotFound(error));
            }
            else
            {
                return Results.BadRequest("BinId must be a Guid");
            }
        }
        [Authorize(Policy = "ExternalSystem")]
        [ProducesResponseType(typeof(List<BinInventoryResponse>), StatusCodes.Status200OK)]
        public static async Task<IResult> OnGetAllBins([FromServices] ISender sender)
        {
                var query = new GetAllInventoryBinsQuery();
                var result = await sender.Send(query);

                return result.Match(
                    onSuccess: (response) => TypedResults.Ok(response),
                    onFailure: (error) => Results.NotFound(error));

        }
    }
}
