cmake_minimum_required(VERSION 3.16)
project(gamecore_inventory CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gamecore_inventory STATIC Catalog.cpp Inventory.cpp)
target_include_directories(gamecore_inventory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

enable_testing()
add_executable(inventory_tests tests/main.cpp tests/catalog_test.cpp tests/shipped_data_test.cpp tests/inventory_test.cpp)
target_compile_definitions(inventory_tests PRIVATE INVENTORY_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

foreach(target gamecore_inventory inventory_tests)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
  endif()
endforeach()
target_include_directories(inventory_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest")
target_link_libraries(inventory_tests PRIVATE gamecore_inventory)
add_test(NAME inventory_tests COMMAND inventory_tests)
