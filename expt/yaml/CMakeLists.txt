cmake_minimum_required(VERSION 3.16)

set (CMAKE_CXX_STANDARD 11)

project(yamltest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(yaml-cpp REQUIRED)

include_directories(${YAML_CPP_INCLUDE_DIR})

add_executable(birth2xattr "")
target_sources(birth2xattr
  PRIVATE
    birth2xattr.cpp)

message("yaml-cpp: ${YAML_CPP_LIBRARY_DIR} ${YAML_CPP_LIBRARIES} ${YAML_CPP_INCLUDE_DIR}")

link_directories(${YAML_CPP_LIBRARY_DIR})
target_link_libraries(birth2xattr
	${YAML_CPP_LIBRARIES}
)
set(CMAKE_VERBOSE_MAKEFILE ON)
