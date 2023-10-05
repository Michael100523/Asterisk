
#include <asterisk.h>
#include <pjsip.h>
#include <pjsip_ua.h>

#include <asterisk/module.h>
#include <asterisk/res_pjsip.h>
#include <asterisk/res_pjsip_session.h>
#include <asterisk/strings.h>

#include "include/res_pjsip_nena_i3.h"

#include <asterisk/logger.h>
#include <asterisk/strings.h>

static const pj_str_t COBRA_NENA_I3_QUEUE_ID_HDR = { "Cobra-Nena-I3-Queue-ID", 22 };
static const pj_str_t CALLINFO_HDR = { "Call-Info", 9 };
static const pj_str_t GEOLOCATION_HDR = { "Geolocation", 11 };

static void dump_nena_i3_ds_data(struct nena_i3_ds_data *data)
{
	struct ast_str *str = ast_str_create(1024);

	ast_str_append(&str, 0, "call_id: %s\n", data->call_id);
	ast_str_append(&str, 0, "incident_id: %s\n", data->incident_id);
	ast_str_append(&str, 0, "queue_id: %s\n", data->queue_id);

	{
		size_t i = 0;

		ast_str_append(&str, 0,  "geo_array: [\n");
		for (i = 0; i < data->geo_array.num_geos; i++) {
			struct nena_geolocation *geo = &data->geo_array.geos[i];
			ast_str_append(&str, 0, "  {\n");
			ast_str_append(&str, 0, "    hvalue: %s\n", geo->hvalue);
			ast_str_append(&str, 0, "    uri: %s\n", geo->uri);
			ast_str_append(&str, 0, "    pidflo:%s\n", geo->pidflo);
			ast_str_append(&str, 0, "  }\n");
		}
		ast_str_append(&str, 0, "]\n");
	}

	{
		size_t i = 0;

		ast_str_append(&str, 0, "ecd_array: [\n");
		for (i = 0; i < data->ecd_array.num_ecds; i++) {
			struct nena_emergency_call_data *ecd = &data->ecd_array.ecds[i];
			ast_str_append(&str, 0, "  {\n");
			ast_str_append(&str, 0, "    hvalue: %s\n", ecd->hvalue);
			ast_str_append(&str, 0, "    purpose: %s\n", ecd->purpose);
			ast_str_append(&str, 0, "    uri:%s\n", ecd->uri);
			ast_str_append(&str, 0, "    content:%s\n", ecd->content);
			ast_str_append(&str, 0, "  }\n");
		}
		ast_str_append(&str, 0, "]\n");
	}

	ast_verb(3, "nena_i3_ds_data:\n%s\n", ast_str_buffer(str));

	ast_free(str);
}

static void nena_i3_ds_data_destroy(void *obj)
{
	struct nena_i3_ds_data *nena_i3_data = obj;

	if (nena_i3_data) {
		if (nena_i3_data->call_id) {
			ast_free(nena_i3_data->call_id);
		}
		if (nena_i3_data->incident_id) {
			ast_free(nena_i3_data->incident_id);
		}
		if (nena_i3_data->queue_id) {
			ast_free(nena_i3_data->queue_id);
		}

		{
			size_t i = 0;
			for (i = 0; i < nena_i3_data->geo_array.num_geos; i++) {
				struct nena_geolocation *geo = &nena_i3_data->geo_array.geos[i];
				if (geo->hvalue) {
					ast_free(geo->hvalue);
				}
				if (geo->uri) {
					ast_free(geo->uri);
				}
				if (geo->pidflo) {
					ast_free(geo->pidflo);
				}
			}
		}

		{
			size_t i = 0;
			for (i = 0; i < nena_i3_data->ecd_array.num_ecds; i++) {
				struct nena_emergency_call_data *ecd = &nena_i3_data->ecd_array.ecds[i];
				if (ecd->hvalue) {
					ast_free(ecd->hvalue);
				}
				if (ecd->purpose) {
					ast_free(ecd->purpose);
				}
				if (ecd->uri) {
					ast_free(ecd->uri);
				}
				if (ecd->content) {
					ast_free(ecd->content);
				}
			}
		}
	
		ast_free(nena_i3_data);
	}
}


static const struct ast_datastore_info nena_i3_ds_info = {
	.type = "nena_i3",
	.destroy = nena_i3_ds_data_destroy,
};


static pjsip_hdr *call_info_hdr_create(pj_pool_t *pool, const char *uri, const char *purpose)
{
	RAII_VAR(struct ast_str *, str, ast_str_create(256), ast_free);
	pj_str_t header_value;

	ast_str_set(&str, 0, "<%s>;purpose=%s", uri, purpose);
	pj_cstr(&header_value, ast_str_buffer(str));

	return (pjsip_hdr *)pjsip_generic_string_hdr_create(pool, &CALLINFO_HDR, &header_value);
}

/* This function was inspired by find_pidf from res_pjsip_geolocation.c */
static int extract_content_by_cid(struct ast_sip_session *session, struct pjsip_rx_data *rdata, char *cid, pj_str_t *content)
{
	/* We got a Content-Identifier (cid), so we're going to search for a element
	 * in the body of the message.  If there's no body, there's no point.  */
	if (!rdata->msg_info.msg->body) {
		return -1;
	}

	/* I3 calls can only have Content-Type set to 'application/sdp' (rarely) or 'multipart/mixed' (most of the time,
	 * if not always). If it's 'multipart/mixed' then we have to find the part that has a Content-ID header value
	 * matching the CID. If it's 'application/sdp', then there is nothing to find. */
	if (!ast_sip_are_media_types_equal(&rdata->msg_info.ctype->media, &pjsip_media_type_multipart_mixed)) {
		return -1;
	}

	pj_str_t cid_str = pj_str(cid);
	pjsip_multipart_part *mp = pjsip_multipart_find_part_by_cid_str(rdata->tp_info.pool, rdata->msg_info.msg->body, &cid_str);

	if (!mp) {
		return -1;
	}

	/* Validating the Content-Type of the body section that has been found would be a good step here, but content
		* types unknown to Asterisk, like I3 Call-Info purposes (ex: EmergencyCallData.ProviderInfo), all get set
		* to 'text/plain', so there is no point trying to validate that. */
	content->ptr = mp->body->data;
	content->slen = mp->body->len;

	return 0;
}

static void extract_queue_id(struct ast_sip_session *session, struct pjsip_rx_data *rdata, struct nena_i3_ds_data *data)
{
	pjsip_generic_string_hdr *hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &COBRA_NENA_I3_QUEUE_ID_HDR, NULL);

	if (hdr) {
		data->queue_id = ast_malloc(hdr->hvalue.slen + 1);
		ast_copy_pj_str(data->queue_id, &hdr->hvalue, hdr->hvalue.slen + 1);
	}
}

static void extract_geolocation(struct ast_sip_session *session, struct pjsip_rx_data *rdata, struct nena_i3_ds_data *nena_i3_data)
{
	pjsip_generic_string_hdr *hdr = NULL;

	for (hdr = NULL; (hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &GEOLOCATION_HDR, hdr)); hdr = hdr->next) {
		pj_str_t hvalue_parse;
		pj_strdup_with_null(rdata->tp_info.pool, &hvalue_parse, &hdr->hvalue);
		char *next_hdr = hvalue_parse.ptr;
		char *cur_hdr = NULL;
		
		while ((cur_hdr = ast_strsep(&next_hdr, ',', AST_STRSEP_TRIM))) {
			if (nena_i3_data->geo_array.num_geos < SIZE_NENA_GEOLOCATION_ARRAY) {
				struct nena_geolocation *geo = &nena_i3_data->geo_array.geos[nena_i3_data->geo_array.num_geos];
				nena_i3_data->geo_array.num_geos++;

				geo->hvalue = ast_strdup(cur_hdr);

				char *uri = strchr(cur_hdr, '<');
				char *tmp = uri ? strchr(cur_hdr, '>') : NULL;

				if (!(uri && tmp)) {
					// malformed Call-Info header
					continue;
				}
				
				uri++;
				tmp[0] = '\0';
				geo->uri = ast_strdup(uri);

				if (strncmp(uri, "cid:", 4) == 0) {
					pj_str_t content = { "", 0 };
					if (extract_content_by_cid(session, rdata, uri, &content) == 0) {
						geo->pidflo = ast_malloc(content.slen + 1);
						ast_copy_pj_str(geo->pidflo, &content, content.slen + 1);
					}
				}
			}
		}
	}
}

static void extract_call_info(struct ast_sip_session *session, struct pjsip_rx_data *rdata, struct nena_i3_ds_data *nena_i3_data)
{
	pjsip_generic_string_hdr *hdr = NULL;

	for (hdr = NULL; (hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &CALLINFO_HDR, hdr)); hdr = hdr->next) {
		pj_str_t hvalue_parse;
		pj_strdup_with_null(rdata->tp_info.pool, &hvalue_parse, &hdr->hvalue);
		char *next_hdr = hvalue_parse.ptr;
		char *cur_hdr = NULL;
		
		while ((cur_hdr = ast_strsep(&next_hdr, ',', AST_STRSEP_TRIM))) {
			char *hvalue = ast_strdupa(cur_hdr); // NOTE: not usually a good idea to do this on the stack
			char *purpose = NULL;
			char *uri = strchr(cur_hdr, '<');
			char *tmp = uri ? strchr(cur_hdr, '>') : NULL;

			if (!(uri && tmp)) {
				// malformed Call-Info header
				continue;
			}

			uri++;
			tmp[0] = '\0';

			char *params = &tmp[1];
			char *param = NULL;

			strsep(&params, ";");
			uri = ast_strip_quoted(uri, "<", ">");

			while ((param = ast_strsep(&params, ';', AST_STRSEP_TRIM))) {
				char *value = param;
				char *name = ast_strsep(&value, '=', AST_STRSEP_TRIM);

				if (strcmp(name, "purpose") == 0) {
					purpose = value ? ast_strip(value) : NULL;
					purpose = purpose ?  ast_strip_quoted(purpose, "\"", "\"") : NULL;
					break;
				}
			}

			if (!ast_strlen_zero(uri) && !ast_strlen_zero(purpose)) {
				if (strcmp(purpose, "emergency-CallId") == 0) {
					if (!nena_i3_data->call_id) {
						nena_i3_data->call_id = ast_strdup(uri);
					}
				}
				else if (strcmp(purpose, "emergency-IncidentId") == 0) {
					if (!nena_i3_data->incident_id) {
						nena_i3_data->incident_id = ast_strdup(uri);
					}
				}
				else if (strncmp(purpose, "EmergencyCallData.", 18) == 0) {
					if (nena_i3_data->ecd_array.num_ecds < SIZE_NENA_ECD_ARRAY) {
						struct nena_emergency_call_data *ecd = &nena_i3_data->ecd_array.ecds[nena_i3_data->ecd_array.num_ecds];
						nena_i3_data->ecd_array.num_ecds++;
						ecd->hvalue = ast_strdup(hvalue);
						ecd->purpose = ast_strdup(purpose);
						ecd->uri = ast_strdup(uri);

						if (strncmp(uri, "cid:", 4) == 0) {
							pj_str_t content = { "", 0 };
							if (extract_content_by_cid(session, rdata, uri, &content) == 0) {
								ecd->content = ast_malloc(content.slen + 1);
								ast_copy_pj_str(ecd->content, &content, content.slen + 1);
							}
						}
					}
				}
			}
		}
	}
}

/* Some code in the following function has been copied from res_pjsip_geolocation.c. */
static int i3_incoming_request(struct ast_sip_session *session, struct pjsip_rx_data *rdata)
{
	RAII_VAR(struct ast_datastore *, datastore, ast_sip_session_get_datastore(session, nena_i3_ds_info.type), ao2_cleanup);
	struct nena_i3_ds_data *data = NULL;

	if (datastore) {
		// datastore is already attached to call
		return 0;
	}

	datastore = ast_sip_session_alloc_datastore(&nena_i3_ds_info, nena_i3_ds_info.type);
	ast_sip_session_add_datastore(session, datastore);

	data = ast_calloc(1, sizeof(*data));
	datastore->data = data;

	// populate data from INVITE headers
	extract_queue_id(session, rdata, data);
	extract_geolocation(session, rdata, data);
	extract_call_info(session, rdata, data);

	dump_nena_i3_ds_data(data);

	return 0;
}

static void i3_outgoing_refer(struct ast_sip_session *session, struct pjsip_tx_data *tdata)
{
	RAII_VAR(struct ast_datastore *, datastore, ast_sip_session_get_datastore(session, nena_i3_ds_info.type), ao2_cleanup);

	if (datastore && datastore->data) {
		// add nena headers to ourgoing request
		struct nena_i3_ds_data *data = datastore->data;

		if (data->call_id) {
			pjsip_msg_add_hdr(tdata->msg, call_info_hdr_create(tdata->pool, data->call_id, "emergency-CallId"));
		}

		if (data->incident_id) {
			pjsip_msg_add_hdr(tdata->msg, call_info_hdr_create(tdata->pool, data->incident_id, "emergency-IncidentId"));
		}
	}
}


/* Start with one ast_sip_session_supplement for all things I3, and split it into multiple supplements
 * if it evolves into something too big, or if clear lines can be drawn between some tasks that
 * the supplement does. */
static struct ast_sip_session_supplement i3_supplement = {
	.method = "INVITE",
	.priority = AST_SIP_SUPPLEMENT_PRIORITY_CHANNEL + 10,
	.incoming_request = i3_incoming_request,
};

static struct ast_sip_session_supplement i3_refer_supplement = {
	.method = "REFER",
	.priority = AST_SIP_SUPPLEMENT_PRIORITY_CHANNEL + 10,
	.outgoing_request = i3_outgoing_refer,
};

static int load_module(void)
{
	ast_sip_session_register_supplement(&i3_supplement);
	ast_sip_session_register_supplement(&i3_refer_supplement);

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	ast_sip_session_unregister_supplement(&i3_supplement);
	ast_sip_session_unregister_supplement(&i3_refer_supplement);
	return 0;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_LOAD_ORDER, "PJSIP NENA I3 Support",
	.support_level = AST_MODULE_SUPPORT_CORE,
	.load = load_module,
	.unload = unload_module,
	.load_pri = AST_MODPRI_APP_DEPEND,
	.requires = "res_pjsip,res_pjsip_session,chan_pjsip",
);
