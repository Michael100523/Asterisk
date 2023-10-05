set(PJSIP_SRC_DIR "${CMAKE_SOURCE_DIR}/third-party/asterisk/third-party/pjproject/source")
set(AST_CUSTOM_INCLUDE_DIRS
	"${CMAKE_SOURCE_DIR}/third-party/asterisk/include"
	"${PJSIP_SRC_DIR}/pjlib/include"
	"${PJSIP_SRC_DIR}/pjlib-util/include"
	"${PJSIP_SRC_DIR}/pjsip/include"
	"${PJSIP_SRC_DIR}/pjmedia/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
set(ASTERISK_MODULES_DIR "/usr/lib/asterisk/modules")

add_subdirectory(res_cas_client)
add_subdirectory(res_cds_publish)
add_subdirectory(res_cms_server)
add_subdirectory(res_pjsip_call_incoming)
add_subdirectory(res_pjsip_cobra)
add_subdirectory(res_pjsip_nena_i3)
