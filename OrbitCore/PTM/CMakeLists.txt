# PTM: Process, Thread, Module
set(PTM_SOURCES
    OrbitModule.h
    OrbitProcess.h
    OrbitThread.h
    ProcessUtils.h
    SymbolUtils.h
    
    OrbitModule.cpp
    OrbitProcess.cpp
    OrbitThread.cpp
    ProcessUtils.cpp
    SymbolUtils.cpp
)

add_library(OrbitPTM STATIC
    ${PTM_SOURCES}
)

target_include_directories(OrbitPTM
PUBLIC
    ${CMAKE_SOURCE_DIR}/OrbitCore
    ${EXTERN_ROOT}/cereal/include
    ${CEREAL_INCLUDE_DIRS}
PRIVATE
    #${BREAKPAD_INCLUDE_DIRS}
)

target_link_libraries(OrbitPTM
    OrbitBase
    #${BREAKPAD_LIBRARIES}
    magic
)
