cmake_minimum_required(VERSION 3.3)
project(rr-thread-pool)

include_directories(BEFORE src)

add_library(tp-lib OBJECT
    src/Cond.cpp
    src/MessageQueue.cpp
    src/Mutex.cpp
    src/Thread.cpp
    src/ThreadPool.cpp
    src/Trace.cpp
    src/Cond.h
    src/Locker.h
    src/Message.h
    src/MessageQueue.h
    src/Mutex.h
    src/Task.h
    src/Thread.h
    src/ThreadPool.h
    src/Trace.h)

add_library(tp-doc INTERFACE)
target_sources(tp-doc INTERFACE doc/Documentation.h)

add_executable(tp-ut
    $<TARGET_OBJECTS:tp-lib>
    test/test_Main.cpp
    test/test_MessageQueue.cpp
    test/test_PI.cpp
    test/test_Thread.cpp
    test/test_ThreadPool.cpp)


find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
	               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	               @ONLY)
	add_custom_target(doc
	                  ${DOXYGEN_EXECUTABLE}
					  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
					  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
					  COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)