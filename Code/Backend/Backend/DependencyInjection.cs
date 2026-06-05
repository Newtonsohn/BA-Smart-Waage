using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.AspNetCore.Authentication.OpenIdConnect;
using Microsoft.IdentityModel.Protocols.OpenIdConnect;
using OpenTelemetry;
using OpenTelemetry.Metrics;
using OpenTelemetry.Resources;
using OpenTelemetry.Trace;
using Polly;
using System.Text.Json;

namespace Backend.API;

public static class DependencyInjection
{
    public static IServiceCollection AddPresentation(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddEndpointsApiExplorer()
        .AddHttpContextAccessor()
        .AddJsonSerializerOptionsForEndpoints()
        .AddProblemDetails()
        .AddAuthenticationPolicies(configuration)
        .AddKeyCloakJwtProvider(configuration)
        .AddHttpClients(configuration)
        .ConfigureOpenTelemetry(configuration);

        return services;
    }

    private static IServiceCollection AddKeyCloakJwtProvider(this IServiceCollection services, IConfiguration configuration)
    {
        var clientId = configuration["Authentication:ClientId"];
        var clientSecret = configuration["Authentication:ClientSecret"];
        var audience = configuration["Authentication:Audience"];
        var validIssuer = configuration["Authentication:Issuer"];
        var authority = configuration["Authentication:Authority"];
        var metadataAddress = configuration["Authentication:MetadataAddress"];
        services.AddAuthentication(options =>
                {
                    options.DefaultScheme = CookieAuthenticationDefaults.AuthenticationScheme;
                    options.DefaultChallengeScheme = OpenIdConnectDefaults.AuthenticationScheme;
                    //options.DefaultAuthenticateScheme = JwtBearerDefaults.AuthenticationScheme;
                })
                .AddJwtBearer(options =>
                    {
                        options.Audience = "account";
                        options.RequireHttpsMetadata = false;
                        options.MetadataAddress = metadataAddress!;
                        options.TokenValidationParameters = new()
                        {
                            ValidateIssuer = true,
                            ValidateAudience = true,
                            ValidAudience = audience,
                            ValidIssuers = [validIssuer]
                        };
                        options.Events = new JwtBearerEvents
                        {
                            OnMessageReceived = context =>
                            {
                                var logger = context.HttpContext.RequestServices.GetRequiredService<ILogger<JwtBearerEvents>>();
                                var token = context.Request.Headers["Authorization"].ToString();
                                if (!string.IsNullOrWhiteSpace(token))
                                {
                                    // Optional: Token kürzen für Log, damit es nicht ewig lang ist
                                    var shortToken = token.Length > 40 ? token[..40] + "..." : token;
                                    logger.LogWarning($"[JWT] Received token: {shortToken}");
                                }
                                return Task.CompletedTask;
                            },
                            OnAuthenticationFailed = context =>
                            {
                                var logger = context.HttpContext.RequestServices.GetRequiredService<ILogger<JwtBearerEvents>>();
                                logger.LogWarning($"[JWT] Token validation failed: {context.Exception.Message}");
                                if (context.Exception.InnerException != null)
                                {
                                    logger.LogError($"[JWT] Inner: {context.Exception.InnerException.Message}");
                                }
                                return Task.CompletedTask;
                            }
                        };
                    })
                .AddCookie(CookieAuthenticationDefaults.AuthenticationScheme)
                .AddOpenIdConnect(options =>
                {
                    options.Authority = authority;
                    options.RequireHttpsMetadata = false;
                    options.ClientId = clientId;
                    options.ClientSecret = clientSecret;
                    options.ResponseType = OpenIdConnectResponseType.Code;
                    options.SaveTokens = true;
                    options.GetClaimsFromUserInfoEndpoint = true;
                    options.CallbackPath = "/signin-oidc";
                });
        return services;
    }
    
    private static IServiceCollection AddAuthenticationPolicies(this IServiceCollection services, IConfiguration configuration)
    {
        var clientId = configuration["Authentication:ClientId"];
        ArgumentException.ThrowIfNullOrEmpty(clientId, nameof(clientId));
        services.AddAuthorization(options =>
        {
            options.AddPolicy("AtLeastEmployee",
                 policy =>
                 {
                     policy.RequireRole("Employee", "Manager", "System-Administrator");
                 });
            options.AddPolicy("AtLeastManager",
                 policy =>
                 {
                     policy.RequireRole("Manager", "System-Administrator");
                 });
            options.AddPolicy("AtLeastSystemAdministrator",
                policy =>
                {
                    policy.RequireRole("System-Administrator");
                });
            options.AddPolicy("Gateway",
               policy =>
               {
                   policy.RequireAuthenticatedUser();
                   policy.AuthenticationSchemes.Clear();
                   policy.AuthenticationSchemes.Add("Bearer");
                   policy.RequireRole("Gateway");
               });
            options.AddPolicy("ExternalSystem",
               policy =>
               {
                   policy.RequireAuthenticatedUser();
                   policy.AuthenticationSchemes.Clear();
                   policy.AuthenticationSchemes.Add("Bearer");
                   policy.RequireRole("ExternalSystem");
               });

        });

        return services;
    }
    
    private static IServiceCollection AddHttpClients(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddHttpClient("GatewayClient")
          .AddTransientHttpErrorPolicy(policyBuilder =>
              policyBuilder.WaitAndRetryAsync(
                  retryCount: 3,
                  sleepDurationProvider: retryAttempt => TimeSpan.FromMilliseconds(Math.Pow(2, retryAttempt) * 100)
              )
      );
        return services;
    }

    private static IServiceCollection ConfigureOpenTelemetry(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddOpenTelemetry()
            .ConfigureResource(resource => resource.AddService("Backend"))
            .WithMetrics(metrics =>
            {
                metrics
                .AddAspNetCoreInstrumentation()
                .AddHttpClientInstrumentation();

                //metrics.AddOtlpExporter();
            })
            .WithTracing(tracing =>
            {
                tracing
                    .AddAspNetCoreInstrumentation()
                    .AddHttpClientInstrumentation()
                    .AddEntityFrameworkCoreInstrumentation();

                //tracing.AddOtlpExporter();
            });

        var useOtlpExporter = !string.IsNullOrWhiteSpace(configuration["OTEL_EXPORTER_OTLP_ENDPOINT"]);
        if (useOtlpExporter)
        {
            services.AddOpenTelemetry().UseOtlpExporter();
        }

        return services;
    }

    private static IServiceCollection AddJsonSerializerOptionsForEndpoints(this IServiceCollection services)
    {
        services.AddControllers().AddJsonOptions(options =>
        {
            options.JsonSerializerOptions.PropertyNameCaseInsensitive = true;
            options.JsonSerializerOptions.PropertyNamingPolicy = JsonNamingPolicy.CamelCase;
        });
        return services;
    }
}
