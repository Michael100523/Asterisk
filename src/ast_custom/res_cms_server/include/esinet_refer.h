#ifndef CMS_SERVER_ESINET_REFER_H_INCLUDED
#define CMS_SERVER_ESINET_REFER_H_INCLUDED

#include <cms_service.h>

#ifdef __cplusplus
extern "C" {
#endif

enum cms_status cms_esinet_refer(struct cms_esinet_refer_req *req, cms_esinet_refer_res_cb res_cb, void *cb_arg);

#ifdef __cplusplus
}
#endif

#endif // CMS_SERVER_ESINET_REFER_H_INCLUDED
