cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-g -DWITH_POSIX -DDEBUG_ENABLE ${CMAKE_C_FLAGS}")

set(COAP_SRC src/async.c
             src/block.c  
             src/coap_list.c  
             src/debug.c  
             src/encode.c  
             src/hashkey.c  
             src/net.c  
             src/option.c  
             src/pdu.c  
             src/resource.c  
             src/str.c  
             src/subscribe.c  
             src/uri.c)

include_directories(./include)

add_library(coap STATIC ${COAP_SRC})

add_executable(coapClient  demo/coapClient.c
                           demo/coapResponse.c
                           demo/coapOpt.c
                           demo/coapURL.c
                           demo/coapNetAddress.c
                           demo/coapPDU.c
                           demo/coapContext.c)
target_link_libraries(coapClient coap)

add_executable(client  example/client.c)
target_link_libraries(client coap)

add_executable(server  example/server.c)
target_link_libraries(server coap)