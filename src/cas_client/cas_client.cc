#include <grpcpp/grpcpp.h>
#include <cas_service.pb.h>
#include <cas_service.grpc.pb.h>

#include "include/cas_client.h"
#include "include/cas_service.h"

using namespace std;

namespace cas {

class CallApplicationServiceClient {
	public:
	CallApplicationServiceClient(shared_ptr<::grpc::Channel> channel, int32_t req_deadline) : _stub(CallApplicationService::NewStub(channel)), _req_deadline(req_deadline) {}

	::grpc::Status AddCall(const AddCallReq &req) {
		::google::protobuf::Empty empty;
		grpc::ClientContext context;
		context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(_req_deadline));

		return _stub->AddCall(&context, req, &empty);
	}

	::grpc::Status GetRouting(const RoutingReq &req, RoutingRes *res) {
		grpc::ClientContext context;
		context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(_req_deadline));

		return _stub->GetRouting(&context, req, res);
	}

	private:
	std::unique_ptr<CallApplicationService::Stub> _stub;
	uint32_t _req_deadline;
};

std::shared_ptr<CallApplicationServiceClient> g_cas_client;

extern "C" {

void cas_client_lib_init(const char *grpc_connection_string, uint32_t req_deadline)
{
	if (!g_cas_client) {
		g_cas_client.reset(new CallApplicationServiceClient(grpc::CreateChannel(grpc_connection_string, grpc::InsecureChannelCredentials()), req_deadline));
	}
}

enum cas_status cas_add_call(struct cas_add_call_req *req)
{
	auto client = g_cas_client;

	if (!client) {
		return CAS_STATUS_UNAVAILABLE;
	}
	else if (!req) {
		return CAS_STATUS_INVALID_ARGUMENT;
	}

	AddCallReq _req;

	if (req->call_id) { _req.set_call_id(req->call_id); }
	if (req->psap_id) { _req.set_psap_id(req->psap_id); }
	if (req->media_server) { _req.set_media_server(req->media_server); }

	grpc::Status status = client->AddCall(_req);

	return static_cast<cas_status>(status.error_code());
}

enum cas_status cas_get_routing(struct cas_routing_req *req, cas_routing_res_cb res_cb, void *arg)
{
	auto client = g_cas_client;

	if (!client) {
		return CAS_STATUS_UNAVAILABLE;
	}
	else if (!req) {
		return CAS_STATUS_INVALID_ARGUMENT;
	}

	RoutingReq _req;
	RoutingRes _res;

	if (req->call_id) { _req.set_call_id(req->call_id); }

	grpc::Status status = client->GetRouting(_req, &_res);

	if (status.error_code() == grpc::StatusCode::OK && res_cb) {
		unique_ptr<const char *> uris;
		struct cas_routing_res res;

		if (_res.has_reject()) {
			res.type = CAS_ROUTING_RES_REJECT;
			res.reject.sip_code = _res.reject().sip_code().c_str();
			res.reject.sip_reason = _res.reject().sip_reason().c_str();
		}
		else {
			res.type = CAS_ROUTING_RES_RING_GROUP;
			res.ring_group.rg_id = _res.ring_group().rg_id().c_str();
			res.ring_group.uris_size = _res.ring_group().uris_size();
			if (res.ring_group.uris_size > 0) {
				uris.reset(new const char *[res.ring_group.uris_size]);
				int i = 0;
				for (i = 0; i < res.ring_group.uris_size; i++) {
					res.ring_group.uris[i] = _res.ring_group().uris(i).c_str();
				}
			}
		}

		res_cb(&res, arg);
	}

	return static_cast<cas_status>(status.error_code());
}

} // extern "C"


} // namespace cds