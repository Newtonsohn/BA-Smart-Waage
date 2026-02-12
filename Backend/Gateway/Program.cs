using EdgeDevice.Authentication;
using EdgeDevice.BLE;
using Polly;
using EdgeDevice.Network;
using EdgeDevice.Register;
using EdgeDevice.Endpoints;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddLogging();
builder.Services.AddHostedService<RegisterGatewayOnStartupService>();
builder.Services.AddTransient<AuthHeaderHandler>();
builder.Services.AddSingleton<GWBackgroundService>();
builder.Services.AddSingleton<IHostedService>(sp => sp.GetRequiredService<GWBackgroundService>());
builder.Services.AddSingleton<IBLEService>(sp => sp.GetRequiredService<GWBackgroundService>());

builder.Services.AddMemoryCache();
builder.Services.AddScoped<IAuthenticationProvider, AuthenticationProvider>();
builder.Services.AddTransient<INetworkInformationProvider, NetworkInformationProvider>();
//builder.Services.AddScoped<IAuthenticationProvider, AuthenticationProviderDummy>();
builder.Services.AddTransient<BleDevice>();
builder.Services.Configure<KeycloakOptions>(builder.Configuration.GetSection(KeycloakOptions.Section));
var backendUrl = builder.Configuration.GetValue<string>("Backend:IpAddress") ?? "https://192.168.2.101:7093";


builder.Services.AddHttpClient("BackendClient", client =>
    {
        client.BaseAddress = new Uri(backendUrl);
        client.DefaultRequestHeaders.Add("Accept", "application/json");
    })
    .AddTransientHttpErrorPolicy(policyBuilder =>
        policyBuilder.WaitAndRetryAsync(
            retryCount: 3,
            sleepDurationProvider: retryAttempt => TimeSpan.FromMilliseconds(Math.Pow(2, retryAttempt) * 100)
        ))
    .ConfigurePrimaryHttpMessageHandler(() =>
    {
        return new HttpClientHandler
        {
            ServerCertificateCustomValidationCallback = HttpClientHandler.DangerousAcceptAnyServerCertificateValidator
        };
    })
    .AddHttpMessageHandler<AuthHeaderHandler>();

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    //app.MapOpenApi();
}

app.UseHttpsRedirection();
app.MapEdgeDevice();
app.Run();

