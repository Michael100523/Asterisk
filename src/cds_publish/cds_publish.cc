#include <grpcpp/grpcpp.h>
#include <call_event.pb.h>
#include <cds_service.pb.h>
#include <cds_service.grpc.pb.h>

#include "include/cds_publish.h"

using namespace std;
using namespace call_event::model;

namespace cds {

class EventPublisherClient {
	public:
	EventPublisherClient(shared_ptr<::grpc::Channel> channel) : _stub(Publisher::NewStub(channel)) {}

	::grpc::Status Publish(const string &uci, CallEvent &call_event) {
		::google::protobuf::Empty empty;
		grpc::ClientContext context;

		CdsEvent cds_event;
		string call_event_str;

		call_event.SerializeToString(&call_event_str);

		cds_event.set_uci(uci);
		cds_event.set_event(call_event_str);

		return _stub->Publish(&context, cds_event, &empty);
	}

	private:
	std::unique_ptr<Publisher::Stub> _stub;
};

std::shared_ptr<EventPublisherClient> g_publisher_client;

void publish_call_event(const string &uci, CallEvent &call_event)
{
	auto publisher_client = g_publisher_client;

	if (publisher_client) {
		publisher_client->Publish(uci, call_event);
	}
}

extern "C" {

void cds_publish_lib_init(const char *grpc_connection_string)
{
	if (!g_publisher_client) {
		g_publisher_client.reset(new EventPublisherClient(grpc::CreateChannel(grpc_connection_string, grpc::InsecureChannelCredentials())));
	}
}

} // extern "C"

} // namespace cds


