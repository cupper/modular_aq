cmake_minimum_required (VERSION 2.6)

project(maq)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(FLAGS
 "-Wall"
 "-Wextra"
 "-Wconversion"
 "-Wcast-qual"
 "-Winline"
 "-Woverloaded-virtual"
 "-Wctor-dtor-privacy"
 "-Wnon-virtual-dtor"
 "-Wold-style-cast"
 "-Wpacked"
 "-Wredundant-decls"
 "-fexceptions"
 "-fPIC"
 "-pipe"
)
foreach(FLAG ${FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endforeach()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

# boost::tti added in boost 1.54
find_package(Boost 1.55 REQUIRED COMPONENTS 
    system
    thread
    chrono
)
find_package(Threads REQUIRED)

set(DEPENDENCIES 
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if(CMAKE_BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif(CMAKE_BUILD_TEST)

install(DIRECTORY ${INCLUDE_DIR}/maq DESTINATION include)
