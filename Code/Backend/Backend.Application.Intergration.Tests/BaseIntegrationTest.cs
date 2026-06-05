using Backend.Infrastructure.Database;
using MediatR;
using Microsoft.Extensions.DependencyInjection;

namespace Backend.Application.Intergration.Tests
{
    public abstract class BaseIntegrationTest: IClassFixture<IntegrationTestWebAppFactory>
    {
        private readonly IServiceScope _scope;
        protected readonly ISender Sender;
        protected readonly ApplicationDbContext DbContext;
        public BaseIntegrationTest(IntegrationTestWebAppFactory factory) {
            _scope = factory.Services.CreateScope();
            Sender = _scope.ServiceProvider.GetRequiredService<ISender>();
            DbContext = _scope.ServiceProvider.GetRequiredService<ApplicationDbContext>();
        }
    }
}
