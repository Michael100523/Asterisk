#include <asterisk.h>
#include <asterisk/module.h>

#include <ast_custom_defs.h>
#include <cds_publish.h>

static enum ast_module_load_result load_module(void)
{
	ast_verb(3, "initializing cds_publish_lib with grpc_connection_string: %s\n", CDS_PUBLISH_DEFAULT_GRPC_CONNECTION_STRING);
	cds_publish_lib_init(CDS_PUBLISH_DEFAULT_GRPC_CONNECTION_STRING);

	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	return 0;
}

AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, "Call Data Services event publisher");
