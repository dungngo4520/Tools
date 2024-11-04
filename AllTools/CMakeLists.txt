set(EXECUTABLES
    calcbase64
    changetime
    checkservicemarkdelete
    curltest
    decompress
    detectarch
    forcedelete
    getinstalledproducts
    hold
    listhandle
    removeupperfilter
    runassystem
    shorttolong
    tooltest
    wifiblock
    win32path
    writefiletest
    writeprocessmemory
)

message(STATUS "Current source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Current binary directory: ${CMAKE_CURRENT_BINARY_DIR}")

foreach(EXECUTABLE ${EXECUTABLES})
    message(STATUS "building ${EXECUTABLE}")
    add_executable(${EXECUTABLE} ${EXECUTABLE}.cpp)
    target_link_libraries(${EXECUTABLE} PRIVATE utils)
    target_compile_definitions(${EXECUTABLE} PRIVATE _UNICODE UNICODE)
    target_compile_options(${EXECUTABLE} PRIVATE /MTd)
endforeach()