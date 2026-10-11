# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/uchat_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/uchat_autogen.dir/ParseCache.txt"
  "uchat_autogen"
  )
endif()
