cmake_minimum_required(VERSION 3.24)

project(AutoPhoto
    VERSION 0.1.0
    DESCRIPTION "Batch image processing desktop app with Qt Quick"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AUTOPHOTO_ENABLE_OPENCV "Enable OpenCV-backed image processing" ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2)

if(AUTOPHOTO_ENABLE_OPENCV)
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    else()
        message(WARNING "OpenCV not found. Building Qt-only preview scaffold.")
    endif()
endif()

qt_standard_project_setup(REQUIRES 6.4)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(qml/components/CoverFlowView.qml PROPERTIES QT_RESOURCE_ALIAS CoverFlowView.qml)
set_source_files_properties(qml/components/ToolPanel.qml PROPERTIES QT_RESOURCE_ALIAS ToolPanel.qml)

qt_add_executable(AutoPhoto
    MANUAL_FINALIZATION
    src/main.cpp
    src/BatchProcessor.cpp
    src/BatchProcessor.h
    src/BoundedBuffer.h
    src/ImageProcessor.cpp
    src/ImageProcessor.h
    src/ImageOrientation.cpp
    src/ImageOrientation.h
    src/ImageListModel.cpp
    src/ImageListModel.h
    src/PreviewController.cpp
    src/PreviewController.h
    src/SshTunnelController.cpp
    src/SshTunnelController.h
    src/ImageCompressor.cpp
    src/ImageCompressor.h
    src/platform/PlatformInterface.h
    src/platform/DesktopPlatform.h
    src/platform/DesktopPlatform.cpp
    src/platform/PlatformFactory.h
    src/platform/PlatformFactory.cpp
)

qt_add_qml_module(autophoto
    URI AutoPhoto
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/CoverFlowView.qml
        qml/components/ToolPanel.qml
)

target_link_libraries(autophoto
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickDialogs2
)

set_target_properties(autophoto PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "AutoPhoto"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.autophoto.batch"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

if(WIN32 OR APPLE)
    set_target_properties(autophoto PROPERTIES OUTPUT_NAME "AutoPhoto")
endif()

if(OpenCV_FOUND)
    target_compile_definitions(autophoto PRIVATE AUTOPHOTO_HAS_OPENCV=1)
    target_include_directories(autophoto PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(autophoto PRIVATE ${OpenCV_LIBS})
endif()

if(WIN32)
    set_target_properties(autophoto PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

set(AUTOPHOTO_YUNET_MODEL "${CMAKE_CURRENT_SOURCE_DIR}/models/face_detection_yunet_2023mar.onnx")
if(EXISTS "${AUTOPHOTO_YUNET_MODEL}")
    add_custom_command(TARGET autophoto POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:autophoto>/models"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AUTOPHOTO_YUNET_MODEL}"
            "$<TARGET_FILE_DIR:autophoto>/models/face_detection_yunet_2023mar.onnx"
    )
    if(APPLE)
        install(FILES "${AUTOPHOTO_YUNET_MODEL}" DESTINATION "AutoPhoto.app/Contents/MacOS/models")
    else()
        install(FILES "${AUTOPHOTO_YUNET_MODEL}" DESTINATION "${CMAKE_INSTALL_BINDIR}/models")
    endif()
else()
    message(WARNING "YuNet model not found at ${AUTOPHOTO_YUNET_MODEL}. Face blur requires this file.")
endif()

install(TARGETS autophoto
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(WIN32 AND OpenCV_FOUND)
    install(CODE "
        file(GLOB OPENCV_DLLS \"$<TARGET_FILE_DIR:autophoto>/opencv_*.dll\")
        if(OPENCV_DLLS)
            file(COPY \${OPENCV_DLLS} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\")
        endif()
    ")
endif()

if(WIN32 OR APPLE)
    qt_generate_deploy_app_script(
        TARGET autophoto
        FILENAME_VARIABLE AUTOPHOTO_DEPLOY_SCRIPT
    )
    install(SCRIPT ${AUTOPHOTO_DEPLOY_SCRIPT})
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/AutoPhoto.iss.in"
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/AutoPhoto.iss"
    @ONLY
)

set(CPACK_PACKAGE_NAME "AutoPhoto")
set(CPACK_PACKAGE_VENDOR "AutoPhoto")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AutoPhoto")
set(CPACK_GENERATOR "ZIP")
if(APPLE)
    list(APPEND CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AutoPhoto")
elseif(WIN32)
    list(APPEND CPACK_GENERATOR "NSIS")
endif()
include(CPack)

qt_finalize_executable(autophoto)
