# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/appTCPsv_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/appTCPsv_autogen.dir/ParseCache.txt"
  "appTCPsv_autogen"
  )
endif()
