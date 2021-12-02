cmake_minimum_required(VERSION 3.0)
project(mvds)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_library(mvds SHARED src/segment/Segment.cpp src/segment/Segment.h
        src/revision/Revision.h src/revision/Revision.cpp
        src/ds/VDataStructure.cpp src/ds/VDataStructure.h
        src/ds/vset/VSet.cpp include/VSet.h src/ds/vqueue/VQueue.cpp
        include/VQueue.h src/ds/vstack/VStack.cpp
        include/VStack.h src/versioned/Versioned.h)
