cmake_minimum_required(VERSION 3.11)

project(fbglExamples C)

add_custom_target(run-examples)

set(FBGL_HEADER "fbgl.h")
set(EXAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

# find freetype2
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE2 REQUIRED freetype2)
include_directories(${FREETYPE2_INCLUDE_DIRS})

function(add_example NAME)
  add_executable("${NAME}" "${EXAMPLES_DIR}/${NAME}.c" "${FBGL_HEADER}")
  target_compile_features("${NAME}" PRIVATE c_std_99)
  target_include_directories("${NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries("${NAME}" PRIVATE ${FREETYPE2_LIBRARIES})
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")
endfunction()

add_example(empty_example)
add_example(line)
add_example(rectangle)
add_example(red)
add_example(texture)
add_example(raw_mode)
add_example(framebuf_info)