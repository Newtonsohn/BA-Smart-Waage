using Backend.Application.Abstractions.Authentication;

namespace Backend.Infrastructure.Authentication
{
    internal class ClientContext(IClientAuthenticationProvider _authenticationProvider) : IClientContext
    {
        public string GetClientAddress()
        {
            return _authenticationProvider.GetCurrentClientAddress();
        }

        public string GetClientId()
        {
            return _authenticationProvider.GetCurrentClientId();
        }
    }
}
