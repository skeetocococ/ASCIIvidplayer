cmake_minimum_required(VERSION 3.10)
project(ASCIIvid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

set(SOURCES main.cpp utils.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# Only link the necessary OpenCV modules
target_link_libraries(${PROJECT_NAME} PRIVATE
    opencv_core
    opencv_highgui
    opencv_videoio
    opencv_imgproc
    opencv_imgcodecs
	 m
)
