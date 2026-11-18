# libjpeg-turbo carries its own old copy of GNUInstallDirs, which installs into lib64 on
# any 64-bit Linux that is not Debian-based. CMake does not search lib64 on Arch, where
# find_package(JPEG) would then skip this copy and take the system libjpeg, whose ABI
# version differs from these headers. Install into lib like every other dep.
set(_jpeg_platform_args "")
set(_jpeg_msvc_runtime_args "")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # WebKitGTK loads the system libjpeg into the same process. Linked with GNU ld, the
    # executable exports the functions of this static copy that a linked shared library
    # references, so code built against the system libjpeg (WebKitGTK, GStreamer's jpeg
    # plugin) would call them and fail their ABI version check. Hidden visibility keeps
    # this copy private to the executable that embeds it.
    set(_jpeg_platform_args -DCMAKE_C_VISIBILITY_PRESET=hidden)
endif ()

if (MSVC)
    # The slicer and its other dependencies use the DLL CRT (/MD). libjpeg-turbo
    # otherwise defaults to the static CRT on Windows, which introduces LIBCMT
    # into the final link and causes LNK4098.
    set(_jpeg_msvc_runtime_args
        -DCMAKE_C_FLAGS_RELEASE=/MD
        -DCMAKE_C_FLAGS_RELWITHDEBINFO=/MD
        -DCMAKE_C_FLAGS_DEBUG=/MDd
        -DCMAKE_CXX_FLAGS_RELEASE=/MD
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=/MD
        -DCMAKE_CXX_FLAGS_DEBUG=/MDd
    )
endif ()

orcaslicer_add_cmake_project(JPEG
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/3.2.0.zip
    URL_HASH SHA256=1b4c2cb6cba1ca5e53b9e85eeeab02154cab4b70894c96e97a9c747a26d19d63
    CMAKE_ARGS
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DWITH_CRT_DLL=ON
        -DCMAKE_INSTALL_LIBDIR=lib
        ${_jpeg_platform_args}
        ${_jpeg_msvc_runtime_args}
)
