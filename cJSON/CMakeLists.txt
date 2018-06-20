add_library(cJSON STATIC cJSON.c)

add_definitions(-std=c++11)
include_directories(./)
add_executable(Demo1_cJSON  Demo/Demo1.cpp)
target_link_libraries(Demo1_cJSON cJSON)
