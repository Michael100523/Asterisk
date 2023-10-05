
#include <asterisk.h>
#include <pjsip.h>
#include <pjsip_ua.h>

#include <asterisk/module.h>
#include <asterisk/res_pjsip.h>
#include <asterisk/res_pjsip_session.h>

#include "include/res_pjsip_cobra.h"


#include <asterisk/logger.h>
#include <asterisk/strings.h>


static const pj_str_t COBRA_CALL_ID = { "Cobra-Call-ID", 13 };
static const pj_str_t COBRA_PSAP_ID = { "Cobra-Psap-ID", 13 };
static const pj_str_t COBRA_PEER_GROUP = { "Cobra-Peer-Group", 16 };

static char *dup_pj_str(pj_str_t *str)
{
	char *buf = ast_malloc(str->slen + 1);
	ast_copy_pj_str(buf, str, str->slen + 1);

	return buf;
}

static void dump_cobra_sip_ds_data(struct cobra_sip_ds_data *data)
{
	struct ast_str *str = ast_str_create(1024);

	ast_str_append(&str, 0, "{\n  call_id: %s\n", data->call_id);
	ast_str_append(&str, 0, "  psap_id: %s\n", data->psap_id);
	ast_str_append(&str, 0, "  peer_group: %s\n}\n", data->peer_group);

	ast_verb(3, "cobra_sip_ds_data:\n%s\n", ast_str_buffer(str));

	ast_free(str);
}

static void cobra_sip_ds_data_destroy(void *obj)
{
	struct cobra_sip_ds_data *data = obj;

	if (data) {
		if (data->call_id) {
			ast_free(data->call_id);
		}
		if (data->psap_id) {
			ast_free(data->psap_id);
		}
		if (data->peer_group) {
			ast_free(data->peer_group);
		}
		ast_free(data);
	}
}


static const struct ast_datastore_info cobra_sip_ds_info = {
	.type = "cobra-sip",
	.destroy = cobra_sip_ds_data_destroy,
};


static int on_incoming_invite_request(struct ast_sip_session *session, struct pjsip_rx_data *rdata)
{
	RAII_VAR(struct ast_datastore *, datastore, ast_sip_session_get_datastore(session, cobra_sip_ds_info.type), ao2_cleanup);
	struct cobra_sip_ds_data *data = NULL;

	if (datastore) {
		// session is already attached to a call
		return 0;
	}

	pjsip_generic_string_hdr *cobra_psap_id_hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &COBRA_PSAP_ID, NULL);
	pjsip_generic_string_hdr *cobra_call_id_hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &COBRA_CALL_ID, NULL);
	pjsip_generic_string_hdr *cobra_peer_group_hdr = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &COBRA_PEER_GROUP, NULL);

	// all calls MUST be associated with a psap
	if (!cobra_psap_id_hdr || cobra_psap_id_hdr->hvalue.slen == 0) {
		ast_sip_session_terminate(session, 403);
		return -1;
	}

	datastore = ast_sip_session_alloc_datastore(&cobra_sip_ds_info, cobra_sip_ds_info.type);
	ast_sip_session_add_datastore(session, datastore);

	data = ast_calloc(1, sizeof(*data));
	datastore->data = data;

	data->psap_id = dup_pj_str(&cobra_psap_id_hdr->hvalue);
	data->call_id = cobra_call_id_hdr ? dup_pj_str(&cobra_call_id_hdr->hvalue) : NULL;
	data->peer_group = cobra_peer_group_hdr ? dup_pj_str(&cobra_peer_group_hdr->hvalue) : NULL;

	dump_cobra_sip_ds_data(data);

	return 0;
}

static struct ast_sip_session_supplement cobra_call_supplement = {
	.method = "INVITE",
	.priority = AST_SIP_SUPPLEMENT_PRIORITY_CHANNEL + 10,
	.incoming_request = on_incoming_invite_request,
};

static int load_module(void)
{
	ast_sip_session_register_supplement(&cobra_call_supplement);

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	ast_sip_session_unregister_supplement(&cobra_call_supplement);
	return 0;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_LOAD_ORDER, "PJSIP Cobra SIP headers support",
	.support_level = AST_MODULE_SUPPORT_CORE,
	.load = load_module,
	.unload = unload_module,
	.load_pri = AST_MODPRI_APP_DEPEND,
	.requires = "res_pjsip,res_pjsip_session,chan_pjsip",
);
