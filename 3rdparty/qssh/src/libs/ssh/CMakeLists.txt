set(QSsh_SRCS
    sshsendfacility.cpp
    sshremoteprocess.cpp
    sshpacketparser.cpp
    sshpacket.cpp
    sshoutgoingpacket.cpp
    sshkeygenerator.cpp
    sshkeyexchange.cpp
    sshincomingpacket.cpp
    sshcryptofacility.cpp
    sshconnection.cpp
    sshchannelmanager.cpp
    sshchannel.cpp
    sshcapabilities.cpp
    sftppacket.cpp
    sftpoutgoingpacket.cpp
    sftpoperation.cpp
    sftpincomingpacket.cpp
    sftpdefs.cpp
    sftpchannel.cpp
    sshremoteprocessrunner.cpp
    sshconnectionmanager.cpp
    sshkeypasswordretriever.cpp
    sftpfilesystemmodel.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty)

add_definitions(-DQSSH_LIBRARY)

# final lib
add_library(QSsh SHARED ${QSsh_SRCS})
set_target_properties(QSsh PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

qt5_use_modules(QSsh Core Network Widgets Concurrent)
target_link_libraries(QSsh Botan)
