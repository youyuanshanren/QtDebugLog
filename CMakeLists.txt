cmake_minimum_required(VERSION 3.16)

project(qmldebug VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()
include_directories(src)

qt_add_executable(appqmldebug
    main.cpp
    src/debug_log_ctrl.cpp
    src/debug_log_ctrl.h
)

qt_add_qml_module(appqmldebug
    URI qmldebug
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(appqmldebug
    PRIVATE Qt6::Quick
)

install(TARGETS appqmldebug
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
