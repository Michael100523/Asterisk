#ifndef CAS_SERVICE_H_INCLUDED
#define CAS_SERVICE_H_INCLUDED

//
// C interface to marshall between Asterisk 'C' code and gRPC 'C++' code
//

#ifdef __cplusplus
extern "C" {
#endif

// matching the gRPC status codes
enum cas_status {
	CAS_STATUS_OK = 0,
	CAS_STATUS_CANCELLED,
	CAS_STATUS_UNKNOWN,
	CAS_STATUS_INVALID_ARGUMENT,
	CAS_STATUS_DEADLINE_EXCEEDED,
	CAS_STATUS_NOT_FOUND,
	CAS_STATUS_ALREADY_EXISTS,
	CAS_STATUS_PERMISSION_DENIED,
	CAS_STATUS_RESOURCE_EXHAUSTED,
	CAS_STATUS_FAILED_PRECONDITION,
	CAS_STATUS_ABORTED,
	CAS_STATUS_OUT_OF_RANGE,
	CAS_STATUS_UNIMPLEMENTED,
	CAS_STATUS_INTERNAL,
	CAS_STATUS_UNAVAILABLE,
	CAS_STATUS_DATA_LOSS,
	CAS_STATUS_UNAUTHENTICATED,
};

//
// AddCall
//
struct cas_add_call_req {
	const char *call_id;
	const char *psap_id;
	const char *media_server;
};

enum cas_status cas_add_call(struct cas_add_call_req *req);


//
// GetRouting
//
struct cas_routing_req {
	const char *call_id;
};

struct cas_route_reject {
	const char *sip_code;
	const char *sip_reason;
};

struct cas_route_ring_group {
	const char *rg_id;
	const char **uris;
	int uris_size;
};

enum cas_routing_res_type {
	CAS_ROUTING_RES_REJECT,
	CAS_ROUTING_RES_RING_GROUP
};

struct cas_routing_res {
	enum cas_routing_res_type type;
	union {
		struct cas_route_reject reject;
		struct cas_route_ring_group ring_group;
	};
};

typedef void (* cas_routing_res_cb)(struct cas_routing_res *res, void *arg);

enum cas_status cas_get_routing(struct cas_routing_req *req, cas_routing_res_cb res_cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif // CAS_SERVICE_H_INCLUDED
