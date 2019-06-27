cmake_minimum_required(VERSION 3.0.0)
project(opencv_test VERSION 0.1.0)

find_package( OpenCV REQUIRED )

include(CTest)
enable_testing()

add_executable(obj_track src/obj_tracking.cpp)
add_executable(obj_track_with_Serial src/obj_tracking_serial.cpp)
add_executable(obj_track_retrack_v1 src/obj_tracking_retrack_v1.cpp)
add_executable(obj_track_retrack_v2 src/obj_tracking_retrack_v2.cpp)
add_executable(obj_track_retrack_v3 src/obj_tracking_retrack_v3.cpp)
add_executable(obj_track_retrack_v4 src/obj_tracking_retrack_v4_camshift.cpp)
add_executable(obj_track_all src/obj_tracking_multi.cpp)
add_executable(obj_track_camshift src/obj_tracking_camshift_Serial.cpp)
add_executable(obj_track_host_remote src/obj_tracking_host_remote.cpp)

include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries(obj_track ${OpenCV_LIBS} )
target_link_libraries(obj_track_with_Serial ${OpenCV_LIBS} )
target_link_libraries(obj_track_retrack_v1 ${OpenCV_LIBS} )
target_link_libraries(obj_track_retrack_v2 ${OpenCV_LIBS} )
target_link_libraries(obj_track_retrack_v3 ${OpenCV_LIBS} )
target_link_libraries(obj_track_retrack_v4 ${OpenCV_LIBS} )
target_link_libraries(obj_track_all ${OpenCV_LIBS} )
target_link_libraries(obj_track_camshift ${OpenCV_LIBS} )
target_link_libraries(obj_track_host_remote ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
