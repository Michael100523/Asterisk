#ifndef RES_PJSIP_COBRA_H_INCLUDED
#define RES_PJSIP_COBRA_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

static const char *cobra_sip_ds_name = "cobra-sip";

struct cobra_sip_ds_data {
	char *call_id;
	char *psap_id;
	char *peer_group; // esinet, admin, agent, siprec, etc
};


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RES_PJSIP_COBRA_H_INCLUDED

