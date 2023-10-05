#include <asterisk.h>
#include <asterisk/module.h>
#include <asterisk/config.h>

#include <ast_custom_defs.h>
#include <cms_server.h>
#include "include/esinet_refer.h"

struct cms_config {
	char *grpc_server_addr;
};

static struct cms_config g_config;

static struct cms_service_impl g_service_impl = {
	.esinet_refer = cms_esinet_refer,
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
		if (!strcasecmp(var->name, "cms_server_grpc_server_addr")) {
			g_config.grpc_server_addr = ast_strdup(var->value);
		}
	}

	if (!g_config.grpc_server_addr) {
		g_config.grpc_server_addr = ast_strdup(CMS_SERVER_DEFAULT_GRPC_SERVER_ADDR);
	}
}

static enum ast_module_load_result load_module(void)
{
	load_config();

	ast_verb(3, "initializing cms_server_lib with grpc_server_addr: %s\n", g_config.grpc_server_addr);
	cms_server_lib_init(g_config.grpc_server_addr, &g_service_impl);

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	if (g_config.grpc_server_addr) {
		ast_free(g_config.grpc_server_addr);
	}
	memset(&g_config, 0, sizeof(g_config));

	return 0;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "Call Media Services server implementation");
