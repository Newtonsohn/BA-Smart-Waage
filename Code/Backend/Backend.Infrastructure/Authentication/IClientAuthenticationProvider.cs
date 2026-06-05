namespace Backend.Infrastructure.Authentication
{
    public interface IClientAuthenticationProvider
    {
        public string GetCurrentClientId();
        public string GetCurrentClientAddress();
    }
}
