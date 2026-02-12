using Backend.Application.Abstractions.Authentication;
using Microsoft.AspNetCore.Http;
using System.Security.Claims;

namespace Backend.Infrastructure.Authentication
{
    internal class AuthenticationProvider(IHttpContextAccessor _httpContextAccessor) : IUserAuthenticationProvider, IClientAuthenticationProvider
    {
        public string GetCurrentUserId()
        {
            if (_httpContextAccessor.HttpContext is null)
            {
                throw new InvalidOperationException("No http context available");
            }
            var user = _httpContextAccessor.HttpContext.User;
            return  GetClaimValue(user, JwTClaims.Subject);
        }

        public UserMedatdata GetCurrentUserMedatdata()
        {
            ArgumentNullException.ThrowIfNull(_httpContextAccessor.HttpContext, nameof(_httpContextAccessor.HttpContext));
            ClaimsPrincipal user = _httpContextAccessor.HttpContext.User;
            
            return new UserMedatdata(
               GetClaimValue(user, JwTClaims.FirstName),
               GetClaimValue(user, JwTClaims.LastName),
               GetClaimValue(user, JwTClaims.Email),
               bool.TryParse(GetClaimValue(user, JwTClaims.EmailVerified), out var emailVerified) && emailVerified
           );
        }

        public string GetCurrentClientId()
        {
            ArgumentNullException.ThrowIfNull(_httpContextAccessor.HttpContext, nameof(_httpContextAccessor.HttpContext));
            ClaimsPrincipal user = _httpContextAccessor.HttpContext.User;
            return GetClaimValue(user, JwTClaims.Subject);  
        }

        public string GetCurrentClientAddress()
        {
            ArgumentNullException.ThrowIfNull(_httpContextAccessor.HttpContext, nameof(_httpContextAccessor.HttpContext));
            ClaimsPrincipal user = _httpContextAccessor.HttpContext.User;
            return GetClaimValue(user, JwTClaims.ClientAddress);
        }

        private static string GetClaimValue(ClaimsPrincipal user, string claimType)
        {
            return user.Claims.FirstOrDefault(c => c.Type == claimType)?.Value
        ?? throw new ArgumentNullException(nameof(claimType), $"Claim '{claimType}' not found.");
        }

    }
}
