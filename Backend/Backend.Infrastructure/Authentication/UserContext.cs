using Backend.Application.Abstractions.Authentication;

namespace Backend.Infrastructure.Authentication
{
    internal class UserContext(IUserAuthenticationProvider _authenticationProvider) : IUserContext
    {

        public Guid GetUserId()
        {
            var id = _authenticationProvider.GetCurrentUserId();
            return Guid.Parse(id);
        }

        public UserMedatdata GetUserMedadata()
        {
            return _authenticationProvider.GetCurrentUserMedatdata();
        }
    }
}
