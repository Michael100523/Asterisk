#
# Sources
#
set(SOURCES
    cas_client.cc
)

#
# Target
#
add_library(cas_client SHARED ${SOURCES})

target_include_directories(cas_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cas_client PRIVATE cas_service)

#
# Install shared library
#
install(TARGETS cas_client LIBRARY)
