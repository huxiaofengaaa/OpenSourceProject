add_definitions(-DMEMWATCH)
include_directories(./include)
add_library(memwatch STATIC src/memwatch.c
                            src/memwatchSetup.c)
