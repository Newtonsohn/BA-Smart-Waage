using FluentValidation;

namespace Backend.Application.Bins.AssignInventoryItem
{
    public class AssginInventoryItemCommandValidator : AbstractValidator<AssignInventoryItemCommand>
    {
        public AssginInventoryItemCommandValidator()
        {
            RuleFor(c => c.Treshold).GreaterThan(0);
            RuleFor(c => c.Capacity).GreaterThan(c => (int)c.Treshold);
        }
    }
}
