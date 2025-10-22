cmake_minimum_required(VERSION 3.16)

project(TCPsv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED SerialPort SerialBus)

qt_add_executable(appTCPsv
    main.cpp
)

qt_add_qml_module(appTCPsv
    URI TCPsv
    VERSION 1.0
    QML_FILES main.qml
    SOURCES TcpServer.h TcpServer.cpp
    SOURCES TcpConnection.h TcpConnection.cpp
    SOURCES
    SOURCES
    QML_FILES ScaleSerial.qml
    SOURCES ScaleInterface232.h ScaleInterface232.cpp
    SOURCES ScaleCommand.h
    SOURCES ClientWorker.h ClientWorker.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appTCPsv PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appTCPsv
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appTCPsv
    PRIVATE Qt6::Quick
    Qt::SerialPort
    Qt::SerialBus
)

include(GNUInstallDirs)
install(TARGETS appTCPsv
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
