namespace Backend.Application.Abstractions.Regex
{
    public static class NetworkRegexPattern
    {
        public static readonly string MacAddressRegexPattern = @"^([0-9A-Fa-f]{2}([-:])){5}([0-9A-Fa-f]{2})$";
        public static readonly string IpAddressRegexPattern = @"^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$";
    }
}
