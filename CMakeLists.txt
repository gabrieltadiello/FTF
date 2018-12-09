cmake_minimum_required (VERSION 2.8.11)
project (FFTProject CXX)

# Setting compilation flags
set(CMAKE_CXX_FLAGS "-pthread -g")

# Setting executables' output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/out)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_HOME_DIRECTORY}/cmake)

# Disable in-source builds 
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

find_package(OpenCV CONFIG REQUIRED)
include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Creating executables
add_executable(NonLocalMeans ${CMAKE_SOURCE_DIR}/src/NonLocalMeans.cpp ${CMAKE_SOURCE_DIR}/src/DisplayImage.cpp)
add_executable(WienerFilter ${CMAKE_SOURCE_DIR}/src/WienerFilter.cpp ${CMAKE_SOURCE_DIR}/src/DisplayImage.cpp)
add_executable(MedianFilter ${CMAKE_SOURCE_DIR}/src/MedianFilter.cpp ${CMAKE_SOURCE_DIR}/src/DisplayImage.cpp)
add_executable(HarmonicMean ${CMAKE_SOURCE_DIR}/src/HarmonicMean.cpp ${CMAKE_SOURCE_DIR}/src/DisplayImage.cpp)

# Linking libraries
target_link_libraries(NonLocalMeans ${OpenCV_LIBS})
target_link_libraries(WienerFilter ${OpenCV_LIBS})
target_link_libraries(MedianFilter ${OpenCV_LIBS})
target_link_libraries(HarmonicMean ${OpenCV_LIBS})
