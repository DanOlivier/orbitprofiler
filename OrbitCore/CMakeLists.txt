set(EXTERNAL_HEADERS
    #${EXTERN_ROOT}/DIA2Dump/Callback.h
    #${EXTERN_ROOT}/DIA2Dump/dia2dump.h
    #${EXTERN_ROOT}/DIA2Dump/PrintSymbol.h
    #${EXTERN_ROOT}/DIA2Dump/regs.h
    ${EXTERN_ROOT}/multicore/common/autoresetevent.h
    ${EXTERN_ROOT}/multicore/common/autoreseteventcondvar.h
    ${EXTERN_ROOT}/multicore/common/benaphore.h
    ${EXTERN_ROOT}/multicore/common/bitfield.h
    ${EXTERN_ROOT}/multicore/common/diningphilosophers.h
    ${EXTERN_ROOT}/multicore/common/inmemorylogger.h
    ${EXTERN_ROOT}/multicore/common/rwlock.h
    ${EXTERN_ROOT}/multicore/common/sema.h
    #${EXTERN_ROOT}/peparse/nt-headers.h
    #${EXTERN_ROOT}/peparse/parse.h
    #${EXTERN_ROOT}/peparse/to_string.h
)

set(EXTERNAL_SOURCES
    #${EXTERN_ROOT}/DIA2Dump/dia2dump.cpp
    #${EXTERN_ROOT}/DIA2Dump/PrintSymbol.cpp
    #${EXTERN_ROOT}/DIA2Dump/regs.cpp
    ${EXTERN_ROOT}/multicore/common/inmemorylogger.cpp
    #${EXTERN_ROOT}/peparse/buffer.cpp
    #${EXTERN_ROOT}/peparse/dump.cpp
    #${EXTERN_ROOT}/peparse/parse.cpp
)

source_group("External\\Header Files" FILES ${EXTERNAL_HEADERS})
source_group("External\\Source Files" FILES ${EXTERNAL_SOURCES})

set(HEADERS
    BlockChain.h
    Capture.h
    ContextSwitch.h
    CoreApp.h
    cvconst.h
    DiaManager.h
    DiaParser.h
    Diff.h
    EventBuffer.h
    EventCallbacks.h
    EventClasses.h
    EventGuid.h
    EventTracer.h
    EventUtils.h
    Hashing.h
    Injection.h
    LogInterface.h
    MemoryTracker.h
    MiniDump.h
    ModuleManager.h
    OrbitDia.h
    OrbitFunction.h
    OrbitModule.h
    OrbitProcess.h
    OrbitRule.h
    OrbitSession.h
    OrbitThread.h
    OrbitType.h
    OrbitUnreal.h
    Params.h
    Pdb.h
    ProcessUtils.h
    RingBuffer.h
    SamplingProfiler.h
    Serialization.h
    ServerTimerManager.h
    SymbolUtils.h
    Tcp.h
    TcpServer.h
    Variable.h
    VariableTracing.h
    Version.h
)

set(SOURCES
    #Callstack.cpp
    Capture.cpp
    #ContextSwitch.cpp
    CoreApp.cpp
    #DiaManager.cpp
    #DiaParser.cpp
    #Diff.cpp
    #EventBuffer.cpp
    #EventCallbacks.cpp
    #EventTracer.cpp
    #EventUtils.cpp
    FunctionStats.cpp
    #Injection.cpp
    LogInterface.cpp
    MemoryTracker.cpp
    MiniDump.cpp
    ModuleManager.cpp
    #OrbitDia.cpp
    #OrbitFunction.cpp
    OrbitModule.cpp
    #OrbitProcess.cpp
    OrbitRule.cpp
    OrbitSession.cpp
    #OrbitThread.cpp
    #OrbitType.cpp
    OrbitUnreal.cpp
    Params.cpp
    Pdb.cpp
    #ProcessUtils.cpp
    #SamplingProfiler.cpp
    Serialization.cpp
    #SymbolUtils.cpp
    Tcp.cpp
    TcpServer.cpp
    #TypeInfo.cpp
    Variable.cpp
    VariableTracing.cpp
    Version.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library(OrbitCore STATIC
	${SOURCES} ${EXTERNAL_SOURCES}
	${HEADERS} ${EXTERNAL_HEADERS}
)

target_include_directories(OrbitCore
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEREAL_INCLUDE_DIR}
PRIVATE
    ${BREAKPAD_INCLUDE_DIR}
)

target_link_libraries(OrbitCore
    OrbitBase
    #dbghelp.lib
    LLVMBinaryFormat
    LLVMDebugInfoCodeView
    LLVMDebugInfoMSF
    LLVMDebugInfoPDB
    LLVMObject
    LLVMObjectYAML
    LLVMSupport
    ${BREAKPAD_LIBRARIES}
)
