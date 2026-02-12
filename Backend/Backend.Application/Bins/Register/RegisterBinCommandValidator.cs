using Backend.Application.Abstractions.Regex;
using FluentValidation;

namespace Backend.Application.Bins.Register
{
    class RegisterBinCommandValidator: AbstractValidator<RegisterBinCommand>
    {
        public RegisterBinCommandValidator()
        {
            RuleFor(device => device.MacAddress)
                .NotEmpty().WithMessage("MAC Address is required.")
                .Matches(NetworkRegexPattern.MacAddressRegexPattern).WithMessage("Invalid MAC Address format. Expected format: AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF");
        }
    }
}
