add_definitions(-std=c++11)

include_directories(./include)

add_library(gtest STATIC src/gtest/gtest.cc
                         src/gtest/gtest-death-test.cc
						 src/gtest/gtest-filepath.cc
						 src/gtest/gtest-port.cc
						 src/gtest/gtest-printers.cc
						 src/gtest/gtest-test-part.cc
						 src/gtest/gtest-typed-test.cc)
target_link_libraries(gtest pthread)
						 
add_library(gmock STATIC src/gmock/gmock-cardinalities.cc
                         src/gmock/gmock-internal-utils.cc
						 src/gmock/gmock-matchers.cc
						 src/gmock/gmock-spec-builders.cc
						 src/gmock/gmock.cc)								  

target_link_libraries(gmock gtest pthread)