namespace EdgeDevice.Authentication
{
    public class AuthenticationProviderDummy : IAuthenticationProvider
    {
        public Task<string> GetAccessToken()
        {
            return Task.FromResult(string.Empty);
        }
    }
}