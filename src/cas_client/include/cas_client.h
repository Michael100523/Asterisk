#ifndef CAS_CLIENT_H_INCLUDED
#define CAS_CLIENT_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

void cas_client_lib_init(const char *grpc_connection_string, uint32_t req_deadline);

#ifdef __cplusplus
}
#endif

#endif // CAS_CLIENT_H_INCLUDED
