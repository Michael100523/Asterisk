
using Azure.Messaging.EventHubs.Producer;


var builder = WebApplication.CreateBuilder(args);

builder.Configuration.AddEnvironmentVariables(prefix: "COBRA_");

string unixSocketPath = builder.Configuration["CdsRelaySockPath"] ?? "/var/run/cds_relay.sock";

if (File.Exists(unixSocketPath))
{
    File.Delete(unixSocketPath);
}

builder.WebHost.ConfigureKestrel(options =>
{
    options.ListenUnixSocket(unixSocketPath);
});

builder.Services.AddGrpc();
builder.Services.AddSingleton(new EventHubProducerClient(builder.Configuration["CdsEventHubConnectionString"] ?? ""));

var app = builder.Build();

app.MapGrpcService<CdsPublisher>();
app.Run();

