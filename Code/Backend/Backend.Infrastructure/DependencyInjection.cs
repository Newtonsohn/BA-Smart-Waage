using Backend.Application.Abstractions.Authentication;
using Backend.Application.Abstractions.Data;
using Backend.Domain.Inventories;
using Backend.Infrastructure.Authentication;
using Backend.Infrastructure.Database;
using Backend.Infrastructure.Inventories;
using Backend.Infrastructure.Outbox;
using Backend.Infrastructure.Outbox.Jobs;
using Hangfire;
using Hangfire.InMemory;
using Infrastructure.Database;
using MediatR;
using Microsoft.AspNetCore.Authorization;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Migrations;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Noser.RAGnRoll.Persistence.Idempotence;

namespace Backend.Infrastructure
{
    public static class DependencyInjection
    {
        public static IServiceCollection AddInfrastructure(this IServiceCollection services, IConfiguration configuration)
        {
            services
                .AddDatabase(configuration)
                .AddMemoryCache()
                .AddAuthenticationPolicies(configuration)
                .AddAuthenticataionProvider()
                .AddAuthenticationContexts()
                .AddBackgroundJobsServer()
                .AddBackgroundJobs()
                .AddIdempodencyEventHandler()
                .AddRepositories();

            return services;
        }

        private static IServiceCollection AddDatabase(this IServiceCollection services, IConfiguration configuration)
        {
            string? connectionString = configuration.GetConnectionString("IndustrialScaleNetDb");
            ArgumentNullException.ThrowIfNull(connectionString, nameof(connectionString));
            services.AddSingleton<ConvertDomainEventToOutboxMessageInterceptor>();
            services.AddDbContext<ApplicationDbContext>((sp, optionsBuilder) =>
            {
            var interceptor = sp.GetRequiredService<ConvertDomainEventToOutboxMessageInterceptor>();
            optionsBuilder.AddInterceptors(interceptor);
                optionsBuilder.UseNpgsql(connectionString, npgsqlOptions =>
                       npgsqlOptions.MigrationsHistoryTable(HistoryRepository.DefaultTableName, Schemas.Default));
                        });
     
            services.AddScoped<IApplicationDbContext>(sp => sp.GetRequiredService<ApplicationDbContext>());

            return services;
        }

        private static IServiceCollection AddAuthenticationPolicies(this IServiceCollection services, IConfiguration configuration)
        {
            services.AddSingleton<IAuthorizationHandler, PermissionAuthorizationHandler>();
            return services;
        }
        
        private static IServiceCollection AddAuthenticataionProvider(this IServiceCollection services)
        {
            services.AddScoped<AuthenticationProvider>();
            services.AddScoped<IClientAuthenticationProvider>(sp => sp.GetRequiredService<AuthenticationProvider>());
            services.AddScoped<IUserAuthenticationProvider>(sp => sp.GetRequiredService<AuthenticationProvider>());
            
            return services;
        }

        private static IServiceCollection AddBackgroundJobsServer(this IServiceCollection services)
        {
   
            services.AddHangfireServer(options =>
            {
                options.ServerName = "smartbin";
                options.SchedulePollingInterval = TimeSpan.FromSeconds(1);
                options.CancellationCheckInterval = TimeSpan.FromSeconds(1);
            });

            services.AddHangfire(config =>
                 config.UseInMemoryStorage(new InMemoryStorageOptions
                 {
                     MaxExpirationTime = TimeSpan.FromHours(0.1),
                 })
            );
            return services;
        }

        private static IServiceCollection AddIdempodencyEventHandler(this IServiceCollection services)
        {
            services.Decorate(typeof(INotificationHandler<>), typeof(IdempotenceDomainEventHandler<>));
            return services;
        }

        private static IServiceCollection AddBackgroundJobs(this IServiceCollection services)
        {
            services.AddTransient<IOutboxProcessorJob, OutboxProcessorJob>();
            services.AddTransient<ICleanUpOutboxFoodPrintJob, CleanUpOutboxFoodPrintJob>();
            return services;
        }

        private static IServiceCollection AddRepositories(this IServiceCollection services)
        {
            services.AddScoped<IInventoryItemStockRepository, InventoryItemStockRepository>();
            return services;
        }
        private static IServiceCollection AddAuthenticationContexts(this IServiceCollection services)
        {
            services.AddScoped<IUserContext, UserContext>();
            services.AddScoped<IClientContext, ClientContext>();
            return services;
        }
    }
}
