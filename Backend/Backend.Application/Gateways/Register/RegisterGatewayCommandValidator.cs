using Backend.Application.Abstractions.Regex;
using FluentValidation;

namespace Backend.Application.Gateways.Register
{
    public class RegisterGatewayCommandValidator : AbstractValidator<RegisterGatewayCommand>
    {
        public RegisterGatewayCommandValidator()
        {
            RuleFor(device => device.MacAddress)
                .NotEmpty().WithMessage("MAC Address is required.")
                .Matches(NetworkRegexPattern.MacAddressRegexPattern).WithMessage("Invalid MAC Address format. Expected format: AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF");

            RuleFor(device => device.IpAddress)
                .NotEmpty().WithMessage("IP Address is required.")
                .Matches(NetworkRegexPattern.IpAddressRegexPattern).WithMessage("Invalid IP Address format. Expected format: 192.168.1.1");
        }
    }
}
