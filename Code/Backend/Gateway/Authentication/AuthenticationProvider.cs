using Microsoft.Extensions.Options;
using System.Text.Json.Serialization;
using System.Text.Json;
using Microsoft.Extensions.Caching.Memory;

namespace EdgeDevice.Authentication
{
    public class AuthenticationProvider : IAuthenticationProvider
    {
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly IMemoryCache _memoryCache;
        private readonly KeycloakOptions _options;
        private readonly ILogger<AuthenticationProvider> _logger;
        private static readonly string TokenCacheKey = "keycloak_token";

        public AuthenticationProvider(
            IHttpClientFactory httpClientFactory,
            IOptions<KeycloakOptions> options,
            ILogger<AuthenticationProvider> logger, 
            IMemoryCache memoryCache)
        {
            _httpClientFactory = httpClientFactory;
            _options = options.Value;
            _memoryCache = memoryCache;
            _logger = logger;
        }

        public async Task<string> GetAccessToken()
        {
            if (_memoryCache.TryGetValue(TokenCacheKey, out string? token))
            {   
                return token!;
            }
            var client = _httpClientFactory.CreateClient();
            client.BaseAddress = new Uri(_options.EndpointUrl);
            var parameters = new Dictionary<string, string>
        {
            { "grant_type", "client_credentials" },
            { "client_id", _options.ClientId },
            { "client_secret", _options.ClientSecret }
        };

            var requestUrl = $"/realms/myrealm/protocol/openid-connect/token";
            var request = new HttpRequestMessage(HttpMethod.Post, requestUrl)
            {
                Content = new FormUrlEncodedContent(parameters)
            };

            var response = await client.SendAsync(request);

            if (!response.IsSuccessStatusCode)
            {
                var error = await response.Content.ReadAsStringAsync();
                throw new Exception($"Failed to get access token: {response.StatusCode} - {error}");
            }

            var content = await response.Content.ReadAsStringAsync();
            try
            {
                var tokenResponse = JsonSerializer.Deserialize<TokenResponse>(content);
                ArgumentNullException.ThrowIfNull(tokenResponse, nameof(tokenResponse));
            
                var expiresIn = tokenResponse?.ExpiresIn ?? 60;
                _logger.LogInformation("Token expires in : {Value}", expiresIn);
                _memoryCache.Set(TokenCacheKey, tokenResponse!.AccessToken, TimeSpan.FromSeconds(expiresIn - 5));
                return tokenResponse?.AccessToken ?? throw new Exception("Token response is null");
            }
            catch(Exception ex)
            {
                _logger.LogError(ex.ToString());
                throw;
            }         
        }

        private class TokenResponse
        {
            [JsonPropertyName("access_token")]
            public string AccessToken { get; set; } = string.Empty;

            [JsonPropertyName("expires_in")]
            public int ExpiresIn { get; set; }
        }
    }

}
