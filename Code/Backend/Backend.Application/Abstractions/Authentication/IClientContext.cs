namespace Backend.Application.Abstractions.Authentication
{
    /// <summary>
    /// Current client context for an authenticated gateway.
    /// </summary>
    public interface IClientContext
    {
        public string GetClientId();
        public string GetClientAddress();

    }
}
