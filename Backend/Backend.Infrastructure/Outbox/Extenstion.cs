using Backend.Infrastructure.Outbox.Jobs;
using Hangfire;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;

namespace Backend.Infrastructure.Outbox;
public static class Extenstion
{
    public static IApplicationBuilder AddReccuringJobs(this WebApplication app)
    {
        app.AddOutboxProcessorJob();
        app.AddOutboxCleanUpJob();
        return app;
    }

    private static IApplicationBuilder AddOutboxProcessorJob(this WebApplication app)
    {
        app.Services.GetRequiredService<IRecurringJobManager>()
            .AddOrUpdate<IOutboxProcessorJob>(
            "outbox-processor",
            job => job.ProcessAsync(CancellationToken.None),
            app.Configuration["BackgroundJobs:Outbox:Schedule"]
            );
        return app;
    }

    private static IApplicationBuilder AddOutboxCleanUpJob(this WebApplication app)
    {
        app.Services.GetRequiredService<IRecurringJobManager>()
            .AddOrUpdate<ICleanUpOutboxFoodPrintJob>(
            "outbox-cleanup",
            job => job.CleanUp(),
            app.Configuration["BackgroundJobs:CleanUp:Schedule"]
            );
        return app;
    }
}
