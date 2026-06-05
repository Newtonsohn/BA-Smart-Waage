# Build stage (x64 Host, ARM64 Target)
# Use this when building on x64 Processor
FROM --platform=linux/amd64 mcr.microsoft.com/dotnet/sdk:9.0.203 AS build

# Use this when building on ARM Processor
# FROM --platform=linux/amd64 mcr.microsoft.com/dotnet/sdk:9.0.203 AS build  

ARG BUILD_CONFIGURATION=Release

WORKDIR /src

# Projektdateien kopieren
COPY ["Gateway/Gateway.csproj", "Gateway/"]
COPY ["Backend.Application/Backend.Application.csproj", "Backend.Application/"]
COPY ["Backend.Domain/Backend.Domain.csproj", "Backend.Domain/"]

# Abhängigkeiten wiederherstellen
RUN dotnet restore "Gateway/Gateway.csproj"

# Gesamten Code kopieren und veröffentlichen
COPY . .
WORKDIR "/src/Gateway"

# Für ARM64 veröffentlichen (self-contained)
RUN dotnet publish "Gateway.csproj" \
    -c $BUILD_CONFIGURATION \
    -r linux-arm64 \
    --self-contained true \
    -o /app/publish \
    /p:PublishTrimmed=false \
    /p:PublishSingleFile=false

# Runtime stage (ARM64, keine Pakete)
# Use this when building on x64 Processor
FROM --platform=linux/arm64 debian:bullseye-slim AS runtime     

# Use this when building on ARM Processor 
# FROM  debian:bullseye-slim AS runtime    

WORKDIR /app

# Keine Paket-Installation unter QEMU!

# Invariant Mode: Keine ICU-Abhängigkeit
ENV DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1

# Code kopieren
COPY --from=build /app/publish .

# Ausführbar machen
RUN chmod +x Gateway

# Startpunkt
ENTRYPOINT ["./Gateway"]
