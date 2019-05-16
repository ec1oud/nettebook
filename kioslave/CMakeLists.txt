cmake_minimum_required(VERSION 3.10)

project(kioslave-ipfs)

configure_file(config-kioslave-ipfs.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-ipfs.h )

set(kio_ipfs_PART_SRCS
ipfs.cpp
)
set(CMAKE_AUTOMOC TRUE)

find_package(Qt5Core REQUIRED Core Network)
find_package(KF5KIO 5.12.0 REQUIRED I18n KIO)
find_package(KF5I18n 5.12.0 REQUIRED I18n)

add_library(kio_ipfs MODULE ${kio_ipfs_PART_SRCS})
target_link_libraries(kio_ipfs Qt5::Network KF5::KIOCore KF5::I18n)

set_target_properties(kio_ipfs PROPERTIES OUTPUT_NAME "ipfs")
set_target_properties(kio_ipfs PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf5/kio")
set_target_properties(kio_ipfs PROPERTIES PREFIX "")
#install(TARGETS kio_ipfs  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio)
# doesn't work, cmake says it doesn't exist even though it does
#install(TARGETS ${CMAKE_SOURCE_DIR}/ipfs.protocol DESTINATION /usr/share/kservices5)
# Ubuntu
#install(TARGETS kio_ipfs  DESTINATION /usr/lib/x86_64-linux-gnu/qt5/plugins/kf5/kio)
# Arch
install(TARGETS kio_ipfs  DESTINATION /usr/lib/qt/plugins/kf5/kio)
