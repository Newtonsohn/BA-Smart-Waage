using System.Net.Http.Headers;

namespace EdgeDevice.Authentication
{
    public class AuthHeaderHandler : DelegatingHandler
    {
        private readonly IAuthenticationProvider _authProvider;

        public AuthHeaderHandler(IAuthenticationProvider authProvider)
        {
            _authProvider = authProvider;
        }

        protected override async Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
        {
            var token = await _authProvider.GetAccessToken();
            request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", token);
            return await base.SendAsync(request, cancellationToken);
        }
    }
}
