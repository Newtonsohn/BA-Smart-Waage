using Microsoft.AspNetCore.Authentication.OpenIdConnect;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace Backend.API.Endpoints
{
    [AllowAnonymous]
    [Route("[controller]")]
    public class AccountController : Controller
    {
        [HttpGet("login")]
        public IActionResult Login(string returnUrl = "/")
        {
            var authProperties = new AuthenticationProperties { RedirectUri = returnUrl };
            return Challenge(authProperties, OpenIdConnectDefaults.AuthenticationScheme);
        }
    }
}
