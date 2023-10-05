#
# Sources
#
set(SOURCES
    cds_publish.cc
)

#
# Target
#
add_library(cds_publish SHARED ${SOURCES})

target_include_directories(cds_publish PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cds_publish PRIVATE cds_service)

#
# Install shared library
#
install(TARGETS cds_publish LIBRARY)
