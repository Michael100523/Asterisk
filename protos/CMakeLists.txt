
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

#
# Call Application Services 'CAS' service
#

# Protobuf/Grpc source files
set(CAS_PROTO_FILES
    cas_service.proto
)

# Add Library target with protobuf sources
add_library(cas_service SHARED ${CAS_PROTO_FILES})
target_link_libraries(cas_service
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cas_service PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Compile protobuf and grpc files in cas_service target to cpp
protobuf_generate(TARGET cas_service LANGUAGE cpp)
protobuf_generate(TARGET cas_service LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Install shared library
install(TARGETS cas_service LIBRARY)



#
# Call Data Services 'CDS' service
#

# Protobuf/Grpc source files
set(CDS_PROTO_FILES
    cds_service.proto
    call_event.proto
)

# Add Library target with protobuf sources
add_library(cds_service SHARED ${CDS_PROTO_FILES})
target_link_libraries(cds_service
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cds_service PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Compile protobuf and grpc files in cds_service target to cpp
protobuf_generate(TARGET cds_service LANGUAGE cpp)
protobuf_generate(TARGET cds_service LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Install shared library
install(TARGETS cds_service LIBRARY)



#
# Call Media Services 'CMS' service
#

# Protobuf/Grpc source files
set(CMS_PROTO_FILES
    cms_service.proto
)

# Add Library target with protobuf sources
add_library(cms_service SHARED ${CMS_PROTO_FILES})
target_link_libraries(cms_service
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(cms_service PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Compile protobuf and grpc files in cms_service target to cpp
protobuf_generate(TARGET cms_service LANGUAGE cpp)
protobuf_generate(TARGET cms_service LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Install shared library
install(TARGETS cms_service LIBRARY)

