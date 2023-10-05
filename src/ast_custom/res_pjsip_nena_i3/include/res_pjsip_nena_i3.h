#ifndef RES_PJSIP_NENA_I3_H_INCLUDED
#define RES_PJSIP_NENA_I3_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// for Geolocation header with PIDF-LO
struct nena_geolocation {
	char *hvalue;
	char *uri;
	char *pidflo;
};

#define SIZE_NENA_GEOLOCATION_ARRAY 16

struct nena_geolocation_array {
	struct nena_geolocation geos[SIZE_NENA_GEOLOCATION_ARRAY];
	size_t num_geos;
};

// for Call-Info headers with purpose EmergencyCallData.xxx
struct nena_emergency_call_data {
	char *hvalue;
	char *purpose;
	char *uri;
	char *content;
};

#define SIZE_NENA_ECD_ARRAY 16

struct nena_emergency_call_data_array {
	struct nena_emergency_call_data ecds[SIZE_NENA_ECD_ARRAY];
	size_t num_ecds;
};

static const char *nena_i3_ds_name = "nena_i3";

struct nena_i3_ds_data {
	char *call_id;
	char *incident_id;
	char *queue_id;
	struct nena_geolocation_array geo_array;
	struct nena_emergency_call_data_array ecd_array;
};


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RES_PJSIP_NENA_I3_H_INCLUDED