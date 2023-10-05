#ifndef CMS_SERVICE_H_INCLUDED
#define CMS_SERVICE_H_INCLUDED


//
// C interface to marshall between Asterisk 'C' code and gRPC 'C++' code
//


#ifdef __cplusplus
extern "C" {
#endif

// matching the gRPC status codes
enum cms_status {
	CMS_STATUS_OK = 0,
	CMS_STATUS_CANCELLED,
	CMS_STATUS_UNKNOWN,
	CMS_STATUS_INVALID_ARGUMENT,
	CMS_STATUS_DEADLINE_EXCEEDED,
	CMS_STATUS_NOT_FOUND,
	CMS_STATUS_ALREADY_EXISTS,
	CMS_STATUS_PERMISSION_DENIED,
	CMS_STATUS_RESOURCE_EXHAUSTED,
	CMS_STATUS_FAILED_PRECONDITION,
	CMS_STATUS_ABORTED,
	CMS_STATUS_OUT_OF_RANGE,
	CMS_STATUS_UNIMPLEMENTED,
	CMS_STATUS_INTERNAL,
	CMS_STATUS_UNAVAILABLE,
	CMS_STATUS_DATA_LOSS,
	CMS_STATUS_UNAUTHENTICATED,
};

//
// EsinetRefer
//
struct cms_esinet_refer_req {
	const char *call_id;
	const char *target_uri;
};

struct cms_esinet_refer_res {
	const char *sip_code;
};

typedef void (* cms_esinet_refer_res_cb)(struct cms_esinet_refer_res *res, void *arg);

//
// Service implementation plugin
//
struct cms_service_impl {
	enum cms_status (* esinet_refer)(struct cms_esinet_refer_req *req, cms_esinet_refer_res_cb res_cb, void *cb_arg);
};

#ifdef __cplusplus
}
#endif

#endif // CMS_SERVICE_H_INCLUDED