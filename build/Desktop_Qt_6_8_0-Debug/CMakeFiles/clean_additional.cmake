# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/TarotCaster_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/TarotCaster_autogen.dir/ParseCache.txt"
  "TarotCaster_autogen"
  )
endif()
