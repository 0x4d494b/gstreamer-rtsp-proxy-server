cmake_minimum_required(VERSION 3.5)
project(rtsp-proxy-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(GSTREAMER_MINIMUM_VERSION 1.0.5)

find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
                               gstreamer-sdp-1.0>=1.4
                               gstreamer-video-1.0>=1.4
                               gstreamer-app-1.0>=1.4
                               gstreamer-rtsp-server-1.0>=1.4)

# Add the include directory
include_directories(
  include
  ${Boost_INCLUDE_DIR}
  ${GST_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

set(LIBS
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${GST_LIBRARIES}
  ${yaml-cpp_LIBRARIES}
  -lpthread
  -lyaml-cpp
)

add_executable(${PROJECT_NAME}
    src/RtspProxyConfig.cpp
    src/RtspProxyProcessor.cpp
    src/RtspServer.cpp
    src/RtspClient.cpp
    src/RtspMedia.cpp
    src/OpenCvReader.cpp
    src/rtsp-proxy-server.cpp
)
target_link_libraries(${PROJECT_NAME} ${LIBS})
