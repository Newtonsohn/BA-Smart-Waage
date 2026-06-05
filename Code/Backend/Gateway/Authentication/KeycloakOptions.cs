namespace EdgeDevice.Authentication
{
    public class KeycloakOptions
    {
        public const string Section = "Keycloak";
        public string EndpointUrl { get; set; } = string.Empty;
        public string ClientId { get; set; } = string.Empty;
        public string ClientSecret { get; set; } = string.Empty;
    }
}
