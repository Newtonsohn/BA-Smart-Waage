namespace Backend.Infrastructure.Authentication
{
    internal static class JwTClaims
    {
        public static readonly string Issuer = "iss";
        public static readonly string Subject = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier";
        public static readonly string EmailVerified = "email_verified";
        public static readonly string FirstName = "given_name";
        public static readonly string LastName = "family_name";
        public static readonly string Email = "email";
        public static readonly string ClientAddress = "clientAddress";
    }
}
