find_package(Qt6 COMPONENTS Widgets WebSockets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(reveller-qt
    connectdialog.cpp
    connectdialog.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    sidheader.cpp
    sidheader.h
    sidinfo.cpp
    sidinfo.h
    siditem.cpp
    siditem.h
    siditemmodel.cpp
    siditemmodel.h
)

target_link_libraries(reveller-qt
    PRIVATE
    Qt6::Widgets
    Qt6::WebSockets
)
