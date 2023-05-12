cmake_minimum_required(VERSION 3.15)

project(my_atm_controller)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wconversion -Wno-sign-conversion -Wpedantic -Wnon-virtual-dtor -Woverloaded-virtual)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

set(${PROJECT_NAME}_SRCS
  src/atm/controller_impl.cpp
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRCS}
)
target_include_directories(${PROJECT_NAME} PUBLIC
  include
)
target_include_directories(${PROJECT_NAME} PRIVATE
  include_private
)

enable_testing()

add_executable(controller_impl_test
  test/src/atm/controller_impl_test.cpp
)
target_link_libraries(controller_impl_test
  my_atm_controller
  GTest::gtest_main
  GTest::gmock_main
)
target_include_directories(controller_impl_test PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include_private
)

include(GoogleTest)
gtest_discover_tests(controller_impl_test)


