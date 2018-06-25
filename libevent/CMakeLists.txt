#add_definitions(-std=c++11)
cmake_minimum_required(VERSION 2.8)
include_directories(include)

set(CORE_SRC src/core/buffer.c
             src/core/bufferevent.c
             src/core/bufferevent_filter.c
             src/core/bufferevent_pair.c
             src/core/bufferevent_ratelim.c
             src/core/bufferevent_sock.c
             src/core/event.c
             src/core/evmap.c
             src/core/evthread.c
             src/core/evutil.c
             src/core/evutil_rand.c
             src/core/evutil_time.c
             src/core/listener.c
             src/core/log.c)

set(SYS_SRC src/sys/strlcpy.c
            src/sys/select.c
            src/sys/poll.c
            src/sys/epoll.c
            src/sys/signal.c)

set(EXTRAS_SRC src/extras/evdns.c
               src/extras/event_tagging.c
               src/extras/evrpc.c
               src/extras/http.c)

## library
add_library(event STATIC ${CORE_SRC} ${SYS_SRC} ${EXTRAS_SRC})
add_library(event_core STATIC ${CORE_SRC} ${SYS_SRC})
add_library(event_extras STATIC ${EXTRAS_SRC} ${SYS_SRC})
add_library(event_pthreads STATIC src/core/evthread_pthread.c)

## sample
add_executable(event_sample-dns-example  sample/dns-example.c)
add_executable(event_sample-read-fifo    sample/event-read-fifo.c)
add_executable(event_sample-hello-world  sample/hello-world.c)
add_executable(event_sample-http-server  sample/http-server.c)
add_executable(event_sample-http-connect sample/http-connect.c)
add_executable(event_sample-signal-test  sample/signal-test.c)
add_executable(event_sample-time-test    sample/time-test.c)

target_link_libraries(event_sample-hello-world event)
target_link_libraries(event_sample-http-server event)
target_link_libraries(event_sample-http-connect event)
target_link_libraries(event_sample-signal-test event)
target_link_libraries(event_sample-dns-example event)
target_link_libraries(event_sample-read-fifo event)
target_link_libraries(event_sample-time-test event)

## test
add_executable(event_test_bench            test/bench.c)
add_executable(event_test_bench_cascade    test/bench_cascade.c)
add_executable(event_test_bench_http       test/bench_http.c)
add_executable(event_test_bench_httpclient test/bench_httpclient.c)
add_executable(event_test_test-changelist  test/test-changelist.c)
add_executable(event_test_test-dumpevents  test/test-dumpevents.c)
add_executable(event_test_test-eof         test/test-eof.c)
add_executable(event_test_test-closed      test/test-closed.c)
add_executable(event_test_test-fdleak      test/test-fdleak.c)
add_executable(event_test_test-init        test/test-init.c)
add_executable(event_test_test-ratelim     test/test-ratelim.c)
add_executable(event_test_test-time        test/test-time.c)
add_executable(event_test_test-weof        test/test-weof.c)

add_executable(event_test_regress          test/regress.c
                                           test/regress.gen.c
                                           test/regress.gen.h
                                           test/regress_buffer.c
                                           test/regress_bufferevent.c
                                           test/regress_dns.c
                                           test/regress_et.c
                                           test/regress_finalize.c
                                           test/regress_http.c
                                           test/regress_listener.c
                                           test/regress_main.c
                                           test/regress_minheap.c
                                           test/regress_rpc.c
                                           test/regress_testutils.c
                                           test/regress_testutils.h
                                           test/regress_util.c
                                           test/tinytest.c
                                           test/regress_thread.c)

target_link_libraries(event_test_bench            event)
target_link_libraries(event_test_bench_cascade    event)
target_link_libraries(event_test_bench_http       event)
target_link_libraries(event_test_bench_httpclient event)
target_link_libraries(event_test_test-changelist  event)
target_link_libraries(event_test_test-dumpevents  event)
target_link_libraries(event_test_test-eof         event)
target_link_libraries(event_test_test-closed      event)
target_link_libraries(event_test_test-fdleak      event)
target_link_libraries(event_test_test-init        event)
target_link_libraries(event_test_test-ratelim     event m)
target_link_libraries(event_test_test-time        event)
target_link_libraries(event_test_test-weof        event)
target_link_libraries(event_test_regress          event pthread event_pthreads)

## demo
add_executable(01_event_signal          demo/01_event_signal.c)
add_executable(02_event_timer           demo/02_event_timer.c)

target_link_libraries(01_event_signal            event)
target_link_libraries(02_event_timer             event)


