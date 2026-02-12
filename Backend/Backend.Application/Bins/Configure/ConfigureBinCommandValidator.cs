using FluentValidation;

namespace Backend.Application.Bins.Configure
{
    public class ConfigureBinCommandValidator: AbstractValidator<ConfigureBinCommand>
    {
        public ConfigureBinCommandValidator()
        {
            

            RuleFor(c => c.UpdateInterval).GreaterThan(0);
            RuleFor(c => c.HeartBeatInterval).GreaterThan(0);
            RuleFor(c => c)
                .Must(c => c.HeartBeatInterval % c.UpdateInterval == 0)
                .WithMessage("Heart beat interval must be a multiple of the update interval.");            
        }
    }
}
