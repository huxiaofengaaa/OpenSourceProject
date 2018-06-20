add_library(cJSON STATIC cJSON.c)

add_definitions(-std=c++11)
include_directories(./)

add_executable(Demo1_cJSON  Demo/Demo1.cpp Demo/Demo0.cpp)
target_link_libraries(Demo1_cJSON cJSON)

add_executable(Demo2_cJSON  Demo/Demo2.cpp Demo/Demo0.cpp)
target_link_libraries(Demo2_cJSON cJSON)

add_executable(Demo3_cJSON  Demo/Demo3.cpp Demo/Demo0.cpp)
target_link_libraries(Demo3_cJSON cJSON)

add_executable(Demo4_cJSON  Demo/Demo4.cpp Demo/Demo0.cpp)
target_link_libraries(Demo4_cJSON cJSON)

add_executable(Demo5_cJSON  Demo/Demo5.cpp Demo/Demo0.cpp)
target_link_libraries(Demo5_cJSON cJSON)

add_executable(Demo6_cJSON  Demo/Demo6.cpp Demo/Demo0.cpp)
target_link_libraries(Demo6_cJSON cJSON)