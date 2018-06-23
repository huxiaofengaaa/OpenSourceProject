add_library(Base64 STATIC src/base64.c)

add_definitions(-std=c++11)
include_directories(./include)

add_executable(Base64_test1  example/test1.c)
target_link_libraries(Base64_test1 Base64)

add_executable(Base64_test2  example/test2.cpp)
target_link_libraries(Base64_test2 Base64)


