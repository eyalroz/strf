cmake_minimum_required (VERSION 2.6)
project (performance_tests)

include_directories(
  "../../../"
  "./fmt-4.0.0"
)

add_definitions(-DFMT_HEADER_ONLY=1)

add_executable(to_char_ptr to_char_ptr.cpp)

install(TARGETS to_char_ptr DESTINATION "./")
