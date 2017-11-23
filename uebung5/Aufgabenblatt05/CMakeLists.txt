cmake_minimum_required(VERSION 2.8)

project(Aufgabenblatt05)

file(GLOB SOURCES "./*.cpp" "./*.h")

# Configure the compiler.
if (${CMAKE_COMPILER_IS_GNUCXX})
        add_definitions(-pedantic -fPIC -DUNIX )
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -D_DEBUG -ggdb")
endif (${CMAKE_COMPILER_IS_GNUCXX})
if (${MSVC})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -DNOMINMAX -EHsc")
endif (${MSVC})

add_executable(${PROJECT_NAME} ${SOURCES})
