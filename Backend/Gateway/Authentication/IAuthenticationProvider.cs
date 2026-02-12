namespace EdgeDevice.Authentication
{
    public interface IAuthenticationProvider
    {
        public Task<string> GetAccessToken();
    }
}