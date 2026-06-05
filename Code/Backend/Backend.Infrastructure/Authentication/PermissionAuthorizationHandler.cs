using Microsoft.AspNetCore.Authorization;
using Microsoft.Extensions.Logging;
using System.Text.Json;

namespace Backend.Infrastructure.Authentication
{
    public class PermissionAuthorizationHandler(ILogger<PermissionAuthorizationHandler> _logger) : AuthorizationHandler<PermissionRequirement>
    {
        protected override Task HandleRequirementAsync(AuthorizationHandlerContext context, PermissionRequirement requirement)
        {
            if (context is null)
            {
                return Task.CompletedTask;
            }

            // Extract "resource_access" claim
            var resourceAccessClaim = context.User.Claims.FirstOrDefault(c => c.Type == "resource_access");

            if (resourceAccessClaim is not null)
            {
                try
                {
                    var resourceAccess = JsonDocument.Parse(resourceAccessClaim.Value);

                    if (resourceAccess.RootElement.TryGetProperty(requirement.Client, out var clientRoles) &&
                        clientRoles.TryGetProperty("roles", out var roles) &&
                        roles.EnumerateArray().Any(r =>  requirement.Permissions.Contains(r.GetString())))
                    {
                        context.Succeed(requirement);
                    }
                    else
                    {
                        context.Fail();
                    }
                }
                catch (JsonException ex)
                {
                    _logger.LogError(ex, "Could not parse jwt from keycloak");
                }
            }

            return Task.CompletedTask;
        }
    }
}
