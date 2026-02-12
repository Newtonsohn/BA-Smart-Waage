using Backend.Domain.Inventories;
using FluentValidation;

namespace Backend.Application.Inventories.Update
{
    internal class UpdateInventoryItemCommandValidator: AbstractValidator<UpdateInventoryItemCommand>   
    {
        public UpdateInventoryItemCommandValidator() {
            RuleFor(c => c.ItemWeight).GreaterThan(0);
            RuleFor(c => c.Treshold).GreaterThanOrEqualTo(2).When(c=> c.Indicator == StockIndicator.Quantity);
            RuleFor(c => c.Treshold).GreaterThan(0).LessThan(100).When(c => c.Indicator == StockIndicator.Percent);
            RuleFor(c => c.ItemName).NotEmpty();
        }
    }
}
