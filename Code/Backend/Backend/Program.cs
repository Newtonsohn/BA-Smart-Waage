using Backend.API;
using Backend.API.Components;
using Backend.API.Endpoints;
using Backend.API.Extensions;
using Backend.Application;
using Backend.Endpoints;
using Backend.Infrastructure;
using Backend.Infrastructure.Database;
using Backend.Infrastructure.Outbox;
using Blazored.Toast;
using Hangfire;
using Microsoft.EntityFrameworkCore;
using Npgsql;

var builder = WebApplication.CreateBuilder(args);

//OpenTelemetry
builder.Logging.AddOpenTelemetry();
builder.Services.AddOpenApi();


builder.Services
    .AddApplication()
    .AddPresentation(builder.Configuration)
    .AddInfrastructure(builder.Configuration);

// Add services to the container.
builder.Services.AddRazorComponents()
    .AddInteractiveServerComponents();

builder.Services.AddAuthorizationBuilder();


//UI
builder.Services.AddBlazoredToast();
builder.Services.AddBlazorBootstrap();

var app = builder.Build();

app.MapLoginAndLogout();

//Outbox reccuring jobs
app.AddReccuringJobs();

await MigrateDbAsync();


// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Error", createScopeForErrors: true);
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
 
}
else
{
    app.UseHangfireDashboard(options: new DashboardOptions
    {
        Authorization = [],
        DarkModeEnabled = true,
    });
    app.MapOpenApi();
    app.UseSwaggerUI(options =>
    {
        options.SwaggerEndpoint("/openapi/v1.json", "OpenAPI V1");
    });
}



app.UseAuthentication();
app.UseAuthorization();

app.UseHttpsRedirection();

app.MapBins();
app.MapGateway();
app.MapInventory();

app.UseAntiforgery();

app.MapStaticAssets();
app.MapRazorComponents<App>()
    .AddInteractiveServerRenderMode();

app.Run();


async Task MigrateDbAsync()
{
    using IServiceScope scope = app.Services.CreateScope();
    await using ApplicationDbContext dbContext = scope.ServiceProvider.GetRequiredService<ApplicationDbContext>();

    // Retry Migration if an NpgsqlException with inner EndOfStreamException occurs or if the
    // exception is a PostgresException with SqlState 57P03 (connection not ready)
    var maxRetries = 6;
    var counter = 0;
    while (counter < maxRetries)
    {
        try
        {
            await dbContext.Database.MigrateAsync();
            break;
        }
        catch (NpgsqlException ex) when (ex.InnerException is EndOfStreamException || ex is PostgresException && ex.SqlState == "57P03")
        {
            await Task.Delay(1000);
            counter++;

            // If the maxRetries is reached, the exception will be thrown
            if (counter == maxRetries)
            {
                throw;
            }
        }
    }
}


public partial class Program { }

