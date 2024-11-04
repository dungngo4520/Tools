file(GLOB_RECURSE SOURCES "*.cpp" "*.h")
add_library(utils STATIC ${SOURCES})
target_compile_definitions(utils PRIVATE _UNICODE UNICODE)
target_compile_options(utils PRIVATE /MTd)
