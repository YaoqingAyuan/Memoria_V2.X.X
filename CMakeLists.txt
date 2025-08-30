# 在CMakeLists.txt开头添加
message(STATUS "------------------ Build Configuration ------------------")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Qt Version: ${QT_VERSION}")
message(STATUS "----------------------------------------------------------")


cmake_minimum_required(VERSION 3.16)

project(MemoriaV2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============== 关键修复 ==============
# 提前包含 GNUInstallDirs，确保变量可用
include(GNUInstallDirs)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ============== FFmpeg 配置 ==============
# 设置 FFmpeg 路径
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/ffmpeg)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    playback_widge.cpp
    playback_widge.h
    dialogs/setting_dialog.cpp
    dialogs/setting_dialog.h
    dialogs/setting_dialog.ui
    dialogs/singleline_import_dialog.cpp
    dialogs/singleline_import_dialog.h
    dialogs/singleline_import_dialog.ui
    data_models/tablecolumns.cpp
    data_models/tablecolumns.h
    data_models/videoitem.cpp
    data_models/videoitem.h
    delegates/progressbardelegate.cpp
    delegates/progressbardelegate.h
    dialogs/del_setting_dialog.h
    dialogs/del_setting_dialog.cpp
    dialogs/del_setting_dialog.ui
    delegates/deletemode.h
    delegates/exportmode.h
    dialogs/export_setting_dialog.h
    dialogs/export_setting_dialog.cpp
    dialogs/export_setting_dialog.ui
)

# 修复嵌套问题：将整个目标创建放在if/else块内
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(MemoriaV2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        data_models/tablemanager.h data_models/tablemanager.cpp
        managers/mergemanager.h managers/mergemanager.cpp
        managers/contextmenumanager.h managers/contextmenumanager.cpp
    )

    # 在FFmpeg配置部分添加
    if(NOT EXISTS ${FFMPEG_DIR}/bin/ffmpeg.exe)
        message(FATAL_ERROR "FFmpeg executable not found at ${FFMPEG_DIR}/bin/ffmpeg.exe")
    endif()
    # 添加复制 FFmpeg 可执行文件的命令
    add_custom_command(TARGET MemoriaV2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${FFMPEG_DIR}/bin/ffmpeg.exe
            $<TARGET_FILE_DIR:MemoriaV2>/ffmpeg.exe
        COMMENT "Copying FFmpeg executable to build directory"
    )

    target_link_libraries(MemoriaV2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Core)

    target_include_directories(MemoriaV2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Qt6 的特定设置
    if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MemoriaV2)
    endif()

    set_target_properties(MemoriaV2 PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    # 添加 FFmpeg 安装指令
    install(FILES ${FFMPEG_DIR}/bin/ffmpeg.exe
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            RENAME ffmpeg.exe)

    # 明确设置许可证文件的目标路径
    install(FILES ${FFMPEG_DIR}/licenses/LICENSE.txt
            DESTINATION "${CMAKE_INSTALL_DOCDIR}"  # 使用引号确保路径正确
            RENAME LICENSE-FFmpeg.txt)

    qt_finalize_executable(MemoriaV2)

else()
    # Qt5 处理
    if(ANDROID)
        add_library(MemoriaV2 SHARED ${PROJECT_SOURCES})
    else()
        add_executable(MemoriaV2 ${PROJECT_SOURCES})
    endif()

    target_link_libraries(MemoriaV2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

    # 添加复制 FFmpeg 的命令（Qt5）
    add_custom_command(TARGET MemoriaV2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${FFMPEG_DIR}/bin
            $<TARGET_FILE_DIR:MemoriaV2>
        COMMENT "Copying FFmpeg binaries to build directory"
    )

    # 添加 FFmpeg 安装指令（Qt5）
    install(DIRECTORY ${FFMPEG_DIR}/bin/
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            USE_SOURCE_PERMISSIONS)

    # 明确设置许可证文件的目标路径
    install(FILES ${FFMPEG_DIR}/licenses/LICENSE.txt
            DESTINATION "${CMAKE_INSTALL_DOCDIR}"
            RENAME LICENSE-FFmpeg.txt)
endif()

# 通用安装配置
install(TARGETS MemoriaV2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
