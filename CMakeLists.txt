cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Library)

## For library
add_subdirectory(googletest)
add_subdirectory(memwatch)
add_subdirectory(cJSON)
add_subdirectory(Base64)
