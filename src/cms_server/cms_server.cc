#include <grpcpp/grpcpp.h>
#include <cms_service.pb.h>
#include <cms_service.grpc.pb.h>

#include "include/cms_server.h"
#include "include/cms_service.h"

using namespace std;

namespace cms {

struct cms_service_impl *g_impl;

extern "C" {
void esinet_refer_res_cb(struct cms_esinet_refer_res *res, void *arg) {
	EsinetReferRes* _res = static_cast<EsinetReferRes *>(arg);
	if (res->sip_code) {
		_res->set_sip_code(res->sip_code);
	}
}
}

class CallMediaServiceImpl : public CallMediaService::Service {
	public:
	grpc::Status EsinetRefer(grpc::ServerContext* context, const EsinetReferReq* req, EsinetReferRes* res) override {
		struct cms_esinet_refer_req _req = {
			.call_id = req->call_id().c_str(),
			.target_uri = req->target_uri().c_str()
		};

		if (g_impl && g_impl->esinet_refer) {
			enum cms_status status = g_impl->esinet_refer(&_req, esinet_refer_res_cb, static_cast<void *>(res));
			return grpc::Status(static_cast<grpc::StatusCode>(status), "");
		}
		else {
			return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Not implemented");
		}
	}
};

std::unique_ptr<grpc::Server> g_cms_server;
std::shared_ptr<CallMediaServiceImpl> g_cms_service;

extern "C" {

void cms_server_lib_init(const char *grpc_server_addr, struct cms_service_impl *impl)
{
	if (!g_cms_server) {
		grpc::ServerBuilder builder;

		builder.AddListeningPort(grpc_server_addr, grpc::InsecureServerCredentials());

		g_cms_service = make_shared<CallMediaServiceImpl>();
		builder.RegisterService(g_cms_service.get());

		g_cms_server = builder.BuildAndStart();
	}

	g_impl = impl;
}

} // extern "C"

} // namespace cms


