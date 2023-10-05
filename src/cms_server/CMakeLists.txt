#
# Sources
#
set(SOURCES
    cms_server.cc
)

#
# Target
#
add_library(cms_server SHARED ${SOURCES})

target_include_directories(cms_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cms_server PRIVATE cms_service)

#
# Install shared library
#
install(TARGETS cms_server LIBRARY)
