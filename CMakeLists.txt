cmake_minimum_required(VERSION 3.16)

project(MEDIASERVICES LANGUAGES C CXX)

add_subdirectory(protos)
add_subdirectory(src/ast_custom)
add_subdirectory(src/cas_client)
add_subdirectory(src/cds_publish)
add_subdirectory(src/cds_relay)
add_subdirectory(src/cms_server)
