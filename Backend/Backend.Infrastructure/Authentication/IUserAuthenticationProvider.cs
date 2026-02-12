using Backend.Application.Abstractions.Authentication;

namespace Backend.Infrastructure.Authentication
{
    public interface IUserAuthenticationProvider
    {
        public string GetCurrentUserId();
        public UserMedatdata GetCurrentUserMedatdata();        
    }
}
