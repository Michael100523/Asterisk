
add_custom_target(cds_relay 
	ALL /usr/bin/dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/cds_relay.csproj --configuration Release -o ${CMAKE_CURRENT_BINARY_DIR}/bin
)

include(GNUInstallDirs)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/systemd/cds_relay.service.in ${CMAKE_CURRENT_BINARY_DIR}/bin/cds_relay.service)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/ DESTINATION ${CMAKE_INSTALL_DATADIR}/cds_relay)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/cds_relay.service DESTINATION /etc/systemd/system/)
