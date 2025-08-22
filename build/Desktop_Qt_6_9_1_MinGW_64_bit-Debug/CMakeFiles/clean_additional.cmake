# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\MemoriaV2_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\MemoriaV2_autogen.dir\\ParseCache.txt"
  "MemoriaV2_autogen"
  )
endif()
