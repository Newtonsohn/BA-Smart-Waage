using Microsoft.AspNetCore.Authorization;

namespace Backend.Infrastructure.Authentication
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="Permission">Permissions to be meet on an OR basis</param>
    /// <param name="Client"></param>
    public record PermissionRequirement(string[] Permissions, string Client) : IAuthorizationRequirement;
}
