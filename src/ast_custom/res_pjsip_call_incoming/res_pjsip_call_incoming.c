
#include <asterisk.h>
#include <pjsip.h>
#include <pjsip_ua.h>
#include <unistd.h>

#include <asterisk/module.h>
#include <asterisk/pbx.h>
#include <asterisk/res_pjsip.h>
#include <asterisk/res_pjsip_session.h>
#include <asterisk/uuid.h>

#include <cas_service.h>
#include <res_pjsip_cobra.h>
#include <res_pjsip_nena_i3.h>

#include "include/publish_cds_call_start.h"
#include "include/publish_cds_call_end.h"

static int publish_cds_call_start_task(void *data)
{
	RAII_VAR(struct ast_sip_session *, session, data, ao2_cleanup);

	if (session) {
		RAII_VAR(struct ast_datastore *, cobra_sip_datastore, ast_sip_session_get_datastore(session, cobra_sip_ds_name), ao2_cleanup);
		RAII_VAR(struct ast_datastore *, nena_i3_datastore, ast_sip_session_get_datastore(session, nena_i3_ds_name), ao2_cleanup);

		struct cobra_sip_ds_data *cobra_sip_data = cobra_sip_datastore ? cobra_sip_datastore->data : NULL;
		struct nena_i3_ds_data *nena_i3_data = nena_i3_datastore ? nena_i3_datastore->data : NULL;

		publish_cds_call_start(cobra_sip_data, nena_i3_data);
	}

	return 0;
}

static int publish_cds_call_end_task(void *data)
{
	RAII_VAR(struct ast_sip_session *, session, data, ao2_cleanup);

	if (session) {
		RAII_VAR(struct ast_datastore *, cobra_sip_datastore, ast_sip_session_get_datastore(session, cobra_sip_ds_name), ao2_cleanup);
		struct cobra_sip_ds_data *cobra_sip_data = cobra_sip_datastore ? cobra_sip_datastore->data : NULL;

		publish_cds_call_end(cobra_sip_data);
	}

	return 0;
}

static int on_incoming_invite_request(struct ast_sip_session *session, struct pjsip_rx_data *rdata)
{
	if (session->inv_session->state == PJSIP_INV_STATE_INCOMING) {
		RAII_VAR(struct ast_datastore *, cobra_sip_datastore, ast_sip_session_get_datastore(session, cobra_sip_ds_name), ao2_cleanup);
		RAII_VAR(struct ast_datastore *, nena_i3_datastore, ast_sip_session_get_datastore(session, nena_i3_ds_name), ao2_cleanup);

		struct cobra_sip_ds_data *cobra_sip_data = cobra_sip_datastore ? cobra_sip_datastore->data : NULL;
		struct nena_i3_ds_data *nena_i3_data = nena_i3_datastore ? nena_i3_datastore->data : NULL;

		if (!cobra_sip_data) {
			ast_log(LOG_ERROR, "No Cobra datastore found on call!\n");
			return 0;
		}

		// TODO: check of the call is a new one or not
		// for now we assume a new call if call_id is not set
		if (ast_strlen_zero(cobra_sip_data->call_id)) {
			cobra_sip_data->call_id = ast_calloc(1, AST_UUID_STR_LEN);
			ast_uuid_generate_str(cobra_sip_data->call_id, AST_UUID_STR_LEN);

			if (session->channel) {
				pbx_builtin_setvar_helper(session->channel, "__Cobra-Call-ID", cobra_sip_data->call_id);
				pbx_builtin_setvar_helper(session->channel, "__Cobra-Psap-ID", cobra_sip_data->psap_id);
			}

			ao2_bump(session);
			ast_sip_push_task(session->serializer, publish_cds_call_start_task, session);
		}
	}

	return 0;
}

static void on_session_end(struct ast_sip_session *session)
{
	RAII_VAR(struct ast_datastore *, cobra_sip_datastore, ast_sip_session_get_datastore(session, cobra_sip_ds_name), ao2_cleanup);

	if (cobra_sip_datastore) {
		ao2_bump(session);
		ast_sip_push_task(session->serializer, publish_cds_call_end_task, session);
	}
}

static struct ast_sip_session_supplement cobra_incoming_call_supplement = {
	.method = "INVITE",
	.priority = AST_SIP_SUPPLEMENT_PRIORITY_CHANNEL + 20,
	.incoming_request = on_incoming_invite_request,
	.session_end = on_session_end,
};

static int load_module(void)
{
	ast_sip_session_register_supplement(&cobra_incoming_call_supplement);

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	ast_sip_session_unregister_supplement(&cobra_incoming_call_supplement);
	return 0;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_LOAD_ORDER, "Cobra incoming call handler",
	.support_level = AST_MODULE_SUPPORT_CORE,
	.load = load_module,
	.unload = unload_module,
	.load_pri = AST_MODPRI_APP_DEPEND,
	.requires = "res_pjsip,res_pjsip_session,chan_pjsip",
);
