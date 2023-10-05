
#include <asterisk.h>
#include <asterisk/app.h>
#include <asterisk/config.h>
#include <asterisk/module.h>
#include <asterisk/pbx.h>
#include <asterisk/strings.h>
#include <asterisk/res_pjsip.h>

#include <ast_custom_defs.h>
#include <cas_client.h>
#include <cas_service.h>

struct cas_config {
	char *grpc_connection_string;
};

static struct cas_config g_config;

static void clear_chan_vars(struct ast_channel *chan, const char *chan_vars[], size_t size_chan_vars)
{
	int i;

	for (i = 0; i < size_chan_vars; i++) {
		pbx_builtin_setvar_helper(chan, chan_vars[i], NULL);
	}
}

static int cas_app_add_call(struct ast_channel *chan, const char *data)
{
	struct cas_add_call_req req;
	const char *chan_vars[] = {
		"CAS_RESULT",
	};

	clear_chan_vars(chan, chan_vars, ARRAY_LEN(chan_vars));

	ast_channel_lock(chan);
	if ((req.call_id = pbx_builtin_getvar_helper(chan, "Cobra-Call-ID"))) {
		req.call_id = ast_strdupa(req.call_id);
	}
	if ((req.psap_id = pbx_builtin_getvar_helper(chan, "Cobra-Psap-ID"))) {
		req.psap_id = ast_strdupa(req.psap_id);
	}
	ast_channel_unlock(chan);

	req.media_server = ast_sip_get_host_ip_string(pj_AF_INET());

	if (cas_add_call(&req) == CAS_STATUS_OK) {
		pbx_builtin_setvar_helper(chan, "CAS_RESULT", "OK");
	}
	else {
		ast_log(LOG_WARNING, "failed to add call for call %s\n", req.call_id ? req.call_id : "");
		pbx_builtin_setvar_helper(chan, "CAS_RESULT", "ERR");
	}

	return 0;
}

static void get_routing_cb(struct cas_routing_res *res, void *arg)
{
	struct ast_channel *chan = arg;

	if (res->type == CAS_ROUTING_RES_REJECT) {
		pbx_builtin_setvar_helper(chan, "CAS_ROUTE", "REJECT");
	}
	else if (res->type == CAS_ROUTING_RES_RING_GROUP) {
		RAII_VAR(struct ast_str *, rg_dial_string, ast_str_create(1024), ast_free);
		int i = 0;

		for (i = 0; i < res->ring_group.uris_size; i++) {
			ast_str_append(&rg_dial_string, 0, "%sPJSIP/agent/%s", (i > 0) ? "&" : "", res->ring_group.uris[i]);
		}

		pbx_builtin_setvar_helper(chan, "CAS_ROUTE", "RING_GROUP");
		pbx_builtin_setvar_helper(chan, "CAS_ROUTE_RG_ID", res->ring_group.rg_id ? res->ring_group.rg_id : "");
		pbx_builtin_setvar_helper(chan, "CAS_ROUTE_RG_DIALSTRING", ast_str_buffer(rg_dial_string));
	}
	else {
		pbx_builtin_setvar_helper(chan, "CAS_ROUTE", "REJECT");
	}
}

static int cas_app_get_routing(struct ast_channel *chan, const char *data)
{
	struct cas_routing_req req;
	const char *chan_vars[] = {
		"CAS_RESULT",
		"CAS_ROUTE", "CAS_ROUTE_RG_ID", "CAS_ROUTE_RG_DIALSTRING"
	};

	clear_chan_vars(chan, chan_vars, ARRAY_LEN(chan_vars));

	ast_channel_lock(chan);
	if ((req.call_id = pbx_builtin_getvar_helper(chan, "Cobra-Call-ID"))) {
		req.call_id = ast_strdupa(req.call_id);
	}
	ast_channel_unlock(chan);

	if (cas_get_routing(&req, get_routing_cb, chan) == CAS_STATUS_OK) {
		pbx_builtin_setvar_helper(chan, "CAS_RESULT", "OK");
	}
	else {
		ast_log(LOG_WARNING, "failed to get routing for call %s\n", req.call_id ? req.call_id : "");
		pbx_builtin_setvar_helper(chan, "CAS_RESULT", "ERR");
	}

	return 0;
}

struct app_reg {
	const char *name;
	int (*exec)(struct ast_channel *, const char *);
};

static struct app_reg apps[] =  {
	{ "CAS_AddCall", cas_app_add_call },
	{ "CAS_GetRouting", cas_app_get_routing },
};

static void load_config(void)
{
	struct ast_flags config_flags = { CONFIG_FLAG_NOCACHE };
	const char *filename = COBRA_CONF_FILE;
	struct ast_config *cfg = ast_config_load(filename, config_flags);
	struct ast_variable *var = NULL;

	if (!cfg || cfg == CONFIG_STATUS_FILEMISSING || cfg == CONFIG_STATUS_FILEINVALID || cfg == CONFIG_STATUS_FILEUNCHANGED) {
		const char *reason = (!cfg ? "unknown error (null config)" : (cfg == CONFIG_STATUS_FILEMISSING ? "file missing" : (cfg == CONFIG_STATUS_FILEINVALID ? "file invalid" : "file unchanged")));
		ast_log(LOG_ERROR, "Failed to load cobra config file '%s': File %s\n", filename, reason);
		return;
	}

	for (var = ast_category_root(cfg, COBRA_CONF_CAT); var; var = var->next) {
		if (!strcasecmp(var->name, "cas_client_grpc_connection_string")) {
			g_config.grpc_connection_string = ast_strdup(var->value);
		}
	}

	if (!g_config.grpc_connection_string) {
		g_config.grpc_connection_string = ast_strdup(CAS_CLIENT_DEFAULT_GRPC_CONNECTION_STRING);
	}
}

static enum ast_module_load_result load_module(void)
{
	load_config();

	ast_verb(3, "initializing cas_client_lib with grpc_connection_string: %s\n", g_config.grpc_connection_string);
	cas_client_lib_init(g_config.grpc_connection_string, CAS_CLIENT_REQ_DEADLINE_MS);

	int i = 0;

	for (i = 0; i < ARRAY_LEN(apps); i++) {
		struct app_reg *app = &apps[i];
		ast_register_application(app->name, app->exec, app->name, app->name);
	}

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	if (g_config.grpc_connection_string) {
		ast_free(g_config.grpc_connection_string);
	}
	memset(&g_config, 0, sizeof(g_config));

	return 0;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "Call Application Services client interface");
