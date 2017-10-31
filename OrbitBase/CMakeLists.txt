set(EXTERNAL_HEADERS
    ${EXTERN_ROOT}/xxHash-r42/xxhash.h
)

set(EXTERNAL_SOURCES
    ${EXTERN_ROOT}/xxHash-r42/xxhash.c
    #${EXTERN_ROOT}/xxHash-r42/xxhsum.c
)

source_group("External\\Header Files" FILES ${EXTERNAL_HEADERS})
source_group("External\\Source Files" FILES ${EXTERNAL_SOURCES})

set(HEADERS
    BaseTypes.h
    Callstack.h
    CallstackTypes.h
    Context.h
    CrashHandler.h
    FunctionArgs.h
    FunctionStats.h
    Log.h
    Message.h
    OrbitAsio.h
    OrbitDbgHelp.h
    Path.h
    PrintVar.h
    Profiling.h
    ScopeTimer.h
    SerializationMacros.h
    TcpEntity.h
    TcpForward.h
    Threading.h
    TimerManager.h
    TypeInfoStructs.h
    Utils.h
)

set(SOURCES
    CallstackPOD.cpp
    #CrashHandler.cpp
    Log.cpp
    Message.cpp
    OrbitAsio.cpp
    Path.cpp
    ScopeTimer.cpp
    TcpEntity.cpp
    TimerManager.cpp
    Utils.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitBase STATIC 
	${SOURCES} ${EXTERNAL_SOURCES}
	${HEADERS} ${EXTERNAL_HEADERS}
)

target_include_directories(OrbitBase
PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXTERN_ROOT}/xxHash-r42
    ${EXTERN_ROOT}/stlsoft-1.9.118/include
    ${EXTERN_ROOT}/concurrentqueue
    ${EXTERN_ROOT}/multicore/common
    ${EXTERN_ROOT}/oqpi/include
    ${DIASDKDir}/include
PRIVATE
    ${BREAKPAD_INCLUDE_DIRS}
)
