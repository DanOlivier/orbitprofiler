#include "Callstack.h"

using namespace std;

//-----------------------------------------------------------------------------
#if _WIN32||_WIN64
__declspec(noinline)
CallStackPOD CallStackPOD::Walk(DWORD64 a_Rip, DWORD64 a_Rsp)
{
    CallStackPOD                  callstack;

#ifdef _WIN64
    CONTEXT                       Context;
    KNONVOLATILE_CONTEXT_POINTERS NvContext;
    UNWIND_HISTORY_TABLE          UnwindHistoryTable;
    PRUNTIME_FUNCTION             RuntimeFunction;
    PVOID                         HandlerData;
    ULONG64                       EstablisherFrame;
    ULONG64                       ImageBase;

    RtlCaptureContext(&Context);

    if (a_Rip != 0)
        Context.Rip = a_Rip;
    if (a_Rsp != 0)
        Context.Rsp = a_Rsp;

    RtlZeroMemory(&UnwindHistoryTable, sizeof(UNWIND_HISTORY_TABLE));

    callstack.m_Data[callstack.m_Depth++] = a_Rip;

    for (ULONG Frame = 0; ; Frame++)
    {
        RuntimeFunction = RtlLookupFunctionEntry(Context.Rip, &ImageBase, &UnwindHistoryTable);

        RtlZeroMemory(&NvContext, sizeof(KNONVOLATILE_CONTEXT_POINTERS));

        if (!RuntimeFunction)
        {
            //
            // If we don't have a RUNTIME_FUNCTION, then we've encountered
            // a leaf function.  Adjust the stack appropriately.
            //

            Context.Rip = (ULONG64)(*(PULONG64)Context.Rsp);
            Context.Rsp += 8;
        }
        else
        {
            RtlVirtualUnwind(UNW_FLAG_NHANDLER
                , ImageBase
                , Context.Rip
                , RuntimeFunction
                , &Context
                , &HandlerData
                , &EstablisherFrame
                , &NvContext);
        }

        if (!Context.Rip)
            break;

        if (callstack.m_Depth < ORBIT_STACK_SIZE)
        {
            callstack.m_Data[callstack.m_Depth++] = Context.Rip;
        }
    }

#else
    callstack = GetCallstackManual(a_Rip, a_Rsp);
#endif

    callstack.Hash();
    return callstack;
}

#endif
