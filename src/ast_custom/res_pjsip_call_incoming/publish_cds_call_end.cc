
#include <string>

#include <call_event.pb.h>
#include <cds_publish.h>
#include <res_pjsip_cobra.h>

#include "include/publish_cds_call_end.h"

namespace cobra {

using namespace std;
using namespace call_event::model;

extern "C" {

void publish_cds_call_end(struct cobra_sip_ds_data *cobra_sip_data)
{
	CallEvent call_event;
	CallEnd *call_end = call_event.mutable_end();

	if (cobra_sip_data) {
		if (cobra_sip_data->call_id) { call_end->set_uci(cobra_sip_data->call_id); }
	}

	cds::publish_call_event(cobra_sip_data->call_id, call_event);
}

} // extern "C"

} // namespace cobra
