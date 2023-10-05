using Azure.Messaging.EventHubs;
using Azure.Messaging.EventHubs.Producer;
using Cds;
using Google.Protobuf.WellKnownTypes;
using Grpc.Core;

public class CdsPublisher : Cds.Publisher.PublisherBase
{
	private readonly ILogger<CdsPublisher> _logger;
	private readonly EventHubProducerClient _producerClient;

	public CdsPublisher(ILogger<CdsPublisher> logger, EventHubProducerClient producerClient)
	{
		_logger = logger;
		_producerClient = producerClient;
	}

	public override async Task<Empty> Publish(CdsEvent request, ServerCallContext context)
	{
		_logger.LogInformation("Processing a grpc Publish request");

		try {
			CreateBatchOptions batchOptions = new CreateBatchOptions();
			batchOptions.PartitionKey = request.Uci;

			var eventBatch = await _producerClient.CreateBatchAsync(batchOptions);
			eventBatch.TryAdd(new EventData(request.Event.ToByteArray()));

			await _producerClient.SendAsync(eventBatch);
		}
		catch {
			// log something or return something?
		}

		return new Empty();
	}
}