#ifndef CDS_PUBLISH_H_INCLUDED
#define CDS_PUBLISH_H_INCLUDED

#ifdef __cplusplus

#include <string>


namespace cds {

using namespace std;
using namespace call_event::model;

void publish_call_event(const string &uci, CallEvent &call_event);

}

#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void cds_publish_lib_init(const char *grpc_connection_string);

#ifdef __cplusplus
}
#endif

#endif // CDS_PUBLISH_H_INCLUDED
