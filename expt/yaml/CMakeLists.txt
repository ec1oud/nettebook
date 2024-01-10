cmake_minimum_required(VERSION 3.16)

project(yamltest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(yaml-cpp REQUIRED)

include_directories(${YAML_INCLUDE_DIRS})

add_executable(birth2xattr "")
target_sources(birth2xattr
  PRIVATE
    birth2xattr.cpp)

message(${YAML_CPP_LIBRARIES})

target_link_libraries(birth2xattr
	${YAML_CPP_LIBRARIES}
)
set(CMAKE_VERBOSE_MAKEFILE ON)
