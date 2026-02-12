using Backend.Domain.Inventories;
using FluentValidation;

namespace Backend.Application.Inventories.Create
{
    internal class CreateInventoryItemCommandValidator: AbstractValidator<CreateInventoryItemCommand>
    {
        public CreateInventoryItemCommandValidator() {
            RuleFor(c => c.ItemWeight).GreaterThan(0);
            RuleFor(c => c.ItemName).NotEmpty();
            RuleFor(c => c.Treshold).GreaterThanOrEqualTo(1).When(c => c.Indicator == StockIndicator.Quantity);
            RuleFor(c => c.Treshold).GreaterThan(0).LessThan(100).When(c => c.Indicator == StockIndicator.Percent);
        }
    }
}
