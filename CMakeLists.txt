cmake_minimum_required(VERSION 3.2)
project(xxx)
ADD_DEFINITIONS(-std=c++17)
include(deps/deps.cmake)
include_directories("src")
include_directories("deps")
set(CMAKE_CXX_FLAGS "-O2 -g -mrtm")
file(GLOB rw_client_SORUCES ""  "./benchs/bench_basic_rw.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc" )
add_executable(rw_client ${rw_client_SORUCES} )

find_library(boost_system_lib NAMES boost_system PATHS ./deps/boost PATH_SUFFIXES lib
               NO_DEFAULT_PATH)

if(NOT boost_system_lib)

	set(boost_system_lib "")

endif()


find_library(boost_coroutine_lib NAMES boost_coroutine PATHS ./deps/boost PATH_SUFFIXES lib
               NO_DEFAULT_PATH)

if(NOT boost_coroutine_lib)

	set(boost_coroutine_lib "")

endif()

target_link_libraries(rw_client pthread gflags ${boost_system_lib} ibverbs ${boost_coroutine_lib} )
file(GLOB bench_insert_SORUCES ""  "./benchs/bench_insert.cc"  "./deps/r2/src/logging.cc" )
add_executable(bench_insert ${bench_insert_SORUCES} )
target_link_libraries(bench_insert pthread gflags )
file(GLOB rw_server_SORUCES ""  "./benchs/basic_rw_server.cc"  "./deps/r2/src/logging.cc" )
add_executable(rw_server ${rw_server_SORUCES} )
target_link_libraries(rw_server pthread ibverbs gflags )
file(GLOB ycsb_server_SORUCES ""  "./benchs/ycsb/server/main.cc"  "./deps/r2/src/logging.cc"  "./benchs/terminate.cc" )
add_executable(ycsb_server ${ycsb_server_SORUCES} )

find_library(mkl_rt_lib NAMES mkl_rt PATHS /opt/intel/mkl PATH_SUFFIXES lib/intel64
               NO_DEFAULT_PATH)

if(NOT mkl_rt_lib)

	set(mkl_rt_lib "")

endif()

target_link_libraries(ycsb_server pthread ${mkl_rt_lib} ${boost_system_lib} ibverbs ${boost_coroutine_lib} gflags )
file(GLOB ycsb_client_SORUCES ""  "./benchs/ycsb/client.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc" )
add_executable(ycsb_client ${ycsb_client_SORUCES} )
target_link_libraries(ycsb_client pthread ${mkl_rt_lib} ${boost_system_lib} ibverbs ${boost_coroutine_lib} gflags )
file(GLOB data_server_SORUCES ""  "./benchs/data_dist/server/main.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc" )
add_executable(data_server ${data_server_SORUCES} )

find_library(jemalloc_lib NAMES jemalloc PATHS ./deps/jemalloc PATH_SUFFIXES lib
               NO_DEFAULT_PATH)

if(NOT jemalloc_lib)

	set(jemalloc_lib "")

endif()

target_link_libraries(data_server pthread ${jemalloc_lib} ${mkl_rt_lib} ${TORCH_LIBRARIES} gflags ${boost_system_lib} ibverbs ${boost_coroutine_lib} )
file(GLOB bench_nn_SORUCES ""  "./benchs/data_dist/bench_nn.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc" )
add_executable(bench_nn ${bench_nn_SORUCES} )
target_link_libraries(bench_nn pthread ${jemalloc_lib} ${mkl_rt_lib} ${TORCH_LIBRARIES} gflags ${boost_system_lib} ibverbs ${boost_coroutine_lib} )
file(GLOB data_client_SORUCES ""  "./benchs/data_dist/client.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc" )
add_executable(data_client ${data_client_SORUCES} )
target_link_libraries(data_client pthread ${TORCH_LIBRARIES} ${mkl_rt_lib} ${boost_system_lib} ibverbs ${boost_coroutine_lib} gflags )
