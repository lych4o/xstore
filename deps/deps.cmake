cmake_minimum_required(VERSION 3.2)
include(ExternalProject)

set(boost_INSTALL_DIR ${CMAKE_SOURCE_DIR}/deps/boost)
ExternalProject_Add(boost
	       URL https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz
               CONFIGURE_COMMAND ./bootstrap.sh  --with-libraries=system,coroutine,serialization --prefix=${boost_INSTALL_DIR}
               BUILD_COMMAND ./b2
               BUILD_IN_SOURCE 1

INSTALL_COMMAND ./b2 install)
add_library( boost_system STATIC IMPORTED )
set_target_properties(boost_system PROPERTIES
                IMPORTED_LOCATION ${boost_INSTALL_DIR}/lib/libboost_system.a)
add_library( boost_coroutine STATIC IMPORTED )
set_target_properties(boost_coroutine PROPERTIES
                IMPORTED_LOCATION ${boost_INSTALL_DIR}/lib/libboost_coroutine.a)
add_library( boost_serialization STATIC IMPORTED )
set_target_properties(boost_serialization PROPERTIES
                IMPORTED_LOCATION ${boost_INSTALL_DIR}/lib/libboost_serialization.a)
include_directories(./deps/boost/include)


set(jemalloc_INSTALL_DIR ${CMAKE_SOURCE_DIR}/deps/jemalloc)
ExternalProject_Add(jemalloc
	       SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/jemalloc
               CONFIGURE_COMMAND ./configure --with-jemalloc-prefix=je --prefix=${jemalloc_INSTALL_DIR}
               BUILD_COMMAND make -j12
               BUILD_IN_SOURCE 1

INSTALL_COMMAND cmake -E echo 'Skipping install step.')
include_directories(./deps/jemalloc/include)

include_directories(/opt/intel/mkl/include)

include_directories(./deps/kvs-workload/.)

include_directories(./deps/tiny-dnn/.)

include_directories(./deps/rlib/..)

include_directories(./deps/ggtest/googletest/include)

include_directories(./deps/libtorch/include/torch/csrc/api/include/)
include_directories(./deps/libtorch/include)
find_package(Torch REQUIRED PATHS /data/share/users/ycli/t/xstore/./deps/libtorch)
include_directories(./deps/boost/include)

include_directories(./deps/r2/.)

include_directories(./deps/jemalloc/include)


set(ggflags_DIR "${CMAKE_SOURCE_DIR}/deps/ggflags")
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/ggflags)
include_directories(./deps/ggflags/include)

set(ggtest_DIR "${CMAKE_SOURCE_DIR}/deps/ggtest")
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/ggtest)
include_directories(./deps/ggtest/googletest/include)

