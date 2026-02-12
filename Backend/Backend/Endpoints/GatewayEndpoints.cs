using Backend.API.Extensions;
using Backend.Application.Abstractions.Data;
using Backend.Application.Gateways.GetAssignedBins;
using Backend.Application.Gateways.Register;
using Backend.Domain.Kernel;
using MediatR;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace Backend.API.Endpoints
{
    public static class GatewayEndpoints
    {
        public static void MapGateway(this IEndpointRouteBuilder endpointRouteBuilder)
        {
            RouteGroupBuilder endpoints = endpointRouteBuilder.MapGroup("gateways").RequireAuthorization();

            endpoints.MapPut("/register", OnRegister)
                .WithSummary("Register a gateway")
                .WithDescription("Registers a gateway device. Returns 201 if created, 200 if already registered, 304 on conflict, 400 on validation error, or 500 on server error.");

            endpoints.MapGet("/assignedBins", OnGetAssignedBins)
                .WithSummary("Get assigned bins for a gateway")
                .WithDescription("Retrieves bin MAC addresses assigned to a gateway by its MAC address.");
        }

        [Authorize(Policy = "Gateway")]
        [ProducesResponseType(StatusCodes.Status201Created)]
        [ProducesResponseType(StatusCodes.Status200OK)]
        [ProducesResponseType(StatusCodes.Status304NotModified)]
        [ProducesResponseType(StatusCodes.Status400BadRequest)]
        [ProducesResponseType(typeof(ProblemDetails), StatusCodes.Status500InternalServerError)]
        private async static Task<IResult> OnRegister(
            [FromBody] RegisterGatewayCommand command,
            [FromServices] ISender sender)
        {
            try
            {
                var result = await sender.Send(command);
                if (result.IsSuccess)
                {
                    if(result.Value)
                    {
                        return Results.StatusCode(201);

                    }
                    {
                        return Results.Ok();
                    }
                }
                else
                {
                    switch(result.Error.Type)
                    {
                        case ErrorType.Conflict: return Results.StatusCode(304);
                        case ErrorType.Validation: return Results.BadRequest();
                        default: return Results.Problem();
                    }
                }
            }
            catch (Exception ex)
            {
                return Results.Problem(ex.Message);
            }
        }

        [Authorize(Policy = "Gateway")]
        [ProducesResponseType(typeof(IEnumerable<string>), StatusCodes.Status200OK)]
        private async static Task<IResult> OnGetAssignedBins(
           [FromQuery] string mac,
           [FromServices] IApplicationDbContext _dbContext,
           [FromServices] ISender _sender)
        {
            var query = new GetAssignedBinMacAddressesQuery(mac);
            var result = await _sender.Send(query);

            return result.MapToHttpResult();
        }
    }
}
