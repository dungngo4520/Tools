file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(utils STATIC ${SOURCES})