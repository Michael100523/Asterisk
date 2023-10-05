#ifndef CMS_SERVER_H_INCLUDED
#define CMS_SERVER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct cms_service_impl;

void cms_server_lib_init(const char *grpc_server_addr, struct cms_service_impl *impl);

#ifdef __cplusplus
}
#endif

#endif // CMS_SERVER_H_INCLUDED
