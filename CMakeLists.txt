cmake_minimum_required(VERSION 2.8)
project( efficientnet-lite0 )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
set( NAME_SRC
    conv.cpp activation.cpp utils.cpp
)
add_executable( run efficientnet-lite0.cpp ${NAME_SRC})
target_link_libraries( run ${OpenCV_LIBS} )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")