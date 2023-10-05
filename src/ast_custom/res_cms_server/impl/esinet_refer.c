#include <asterisk.h>
#include <asterisk/channel.h>
#include <asterisk/res_pjsip.h>
#include <asterisk/res_pjsip_session.h>

#include <res_pjsip_cobra.h>
#include <esinet_refer.h>

static int find_esinet_chan(void *obj, void *arg, void *data, int flags)
{
	struct ast_channel *chan = obj;
	const char *call_id = arg;

	if (!call_id) {
		return CMP_STOP;
	}
	else if (!chan || strcmp(ast_channel_tech(chan)->type, "PJSIP") != 0) {
		return 0;
	}

	struct ast_sip_channel_pvt *channel = ast_channel_tech_pvt(chan);

	if (!channel || !channel->session) {
		return 0;
	}

	RAII_VAR(struct ast_datastore *, cobra_sip_datastore, ast_sip_session_get_datastore(channel->session, cobra_sip_ds_name), ao2_cleanup);
	struct cobra_sip_ds_data *cobra_sip_data = cobra_sip_datastore ? cobra_sip_datastore->data : NULL;

	if (cobra_sip_data && cobra_sip_data->call_id && cobra_sip_data->peer_group) {
		return (strcmp(cobra_sip_data->call_id, call_id) == 0 && strcmp(cobra_sip_data->peer_group, "esinet") == 0) ? CMP_MATCH : 0;
	}

	return 0;
}

enum cms_status cms_esinet_refer(struct cms_esinet_refer_req *req, cms_esinet_refer_res_cb res_cb, void *cb_arg)
{
	enum cms_status status = CMS_STATUS_OK;
	struct cms_esinet_refer_res refer_res = {0};
	RAII_VAR(struct ast_channel *, chan, NULL, ao2_cleanup);
	char sip_code_buf[32];

	ast_verb(4, "%s: callid='%s', target_uri='%s' ", __func__, req->call_id, req->target_uri);

	// TODO: should validate the target_uri format since it will be injected into a SIP header

	// find the call and initiate an attented transfer
	// TODO: this is horribly inefficient, but is only a temporary implementation
	if (!(chan = ast_channel_callback(find_esinet_chan, (void *)req->call_id, NULL, 0))) {
		return CMS_STATUS_NOT_FOUND;
	}

	{
		SCOPED_CHANNELLOCK(chan_lock, chan);

		if (ast_test_flag(ast_channel_flags(chan), AST_FLAG_ZOMBIE) || ast_check_hangup(chan)) {
			// channel is basically gone
			return CMS_STATUS_NOT_FOUND;
		}

		char *target_uri = (char *)req->target_uri;
		if (req->target_uri[0] != '<') {
			target_uri = ast_alloca(strlen(req->target_uri)+3);
			snprintf(target_uri, strlen(req->target_uri)+3, "<%s>", req->target_uri);
		}

		if (!ast_channel_tech(chan)->transfer || ast_channel_tech(chan)->transfer(chan, target_uri) != 0) {
			return CMS_STATUS_INTERNAL;
		}
	}

	refer_res.sip_code = "200";
	res_cb(&refer_res, cb_arg);

	return CMS_STATUS_OK;
}
