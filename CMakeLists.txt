cmake_minimum_required(VERSION 3.0)
project(mvds)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_library(mvds SHARED src/segment/Segment.cpp src/segment/Segment.h
        src/revision/Revision.h src/revision/Revision.cpp
        src/VDataStructure.h include/VQueue.h
        include/VStack.h src/versioned/Versioned.h include/VBasicType.h)

add_executable(tests tests/VBasicTypeTests.cpp
        tests/VQueueTests.cpp
        tests/VStackTests.cpp
        tests/tests.cpp)
target_link_libraries(tests mvds -lgtest -lgtest_main -lpthread)
