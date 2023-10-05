
set(AST_MODULE_NAME res_cds_publish)

set(SOURCES
    res_cds_publish.c
)

add_library(${AST_MODULE_NAME} SHARED ${SOURCES})
set_target_properties(${AST_MODULE_NAME} PROPERTIES PREFIX "")

target_include_directories(${AST_MODULE_NAME} PRIVATE ${AST_CUSTOM_INCLUDE_DIRS})
target_include_directories(${AST_MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${AST_MODULE_NAME} PRIVATE cds_service)
target_link_libraries(${AST_MODULE_NAME} PRIVATE cds_publish)
target_compile_definitions(${AST_MODULE_NAME} PUBLIC AST_MODULE=\"${AST_MODULE_NAME}\" AST_MODULE_SELF_SYM=__internal_${AST_MODULE_NAME}_self)

install(TARGETS ${AST_MODULE_NAME} DESTINATION ${ASTERISK_MODULES_DIR})

