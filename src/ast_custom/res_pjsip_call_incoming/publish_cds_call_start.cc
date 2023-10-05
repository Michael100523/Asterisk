
#include <string>

#include <call_event.pb.h>
#include <cds_publish.h>
#include <res_pjsip_cobra.h>
#include <res_pjsip_nena_i3.h>

#include "include/publish_cds_call_start.h"

namespace cobra {

using namespace std;
using namespace call_event::model;

extern "C" {

void publish_cds_call_start(struct cobra_sip_ds_data *cobra_sip_data, struct nena_i3_ds_data *nena_i3_data)
{
	CallEvent call_event;
	CallStart *call_start = call_event.mutable_start();

	if (cobra_sip_data) {
		if (cobra_sip_data->call_id) { call_start->set_uci(cobra_sip_data->call_id); }
		if (cobra_sip_data->psap_id) { call_start->set_psap_id(cobra_sip_data->psap_id); }
		call_start->set_call_direction(CallDirection::CALL_DIRECTION_INCOMING);
	}

	if (nena_i3_data) {
		EmergencyCallData *ecd = call_start->mutable_emergency_call_data();

		if (nena_i3_data->call_id) { ecd->set_emergency_call_id(nena_i3_data->call_id); }
		if (nena_i3_data->incident_id) { ecd->set_emergency_incident_id(nena_i3_data->incident_id); }
		if (nena_i3_data->queue_id) { ecd->set_emergency_queue_id(nena_i3_data->queue_id); }

		if (nena_i3_data->geo_array.num_geos > 0) {
			struct nena_geolocation *geo = &nena_i3_data->geo_array.geos[0];
			if (geo->uri && strncmp(geo->uri, "cid:", 4) != 0) { ecd->set_geolocation_uri(geo->uri); }
			if (geo->pidflo) { ecd->set_pidflo_value(geo->pidflo); }
		}

		size_t i = 0;
		for (i = 0; i < nena_i3_data->ecd_array.num_ecds; i++) {
			struct nena_emergency_call_data *ecd_data = &nena_i3_data->ecd_array.ecds[i];

			if (ecd_data->uri) {
				if (strncmp(ecd_data->uri, "cid:", 4) == 0) {
					if (ecd_data->content) {
						ecd->add_emergency_call_data_values(ecd_data->content);
					}
				}
				else {
					ecd->add_emergency_call_data_uris(ecd_data->uri);
				}
			}
		}
	}

	cds::publish_call_event(cobra_sip_data->call_id, call_event);
}

} // extern "C"

} // namespace cobra
