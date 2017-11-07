#include <Injection.h>
#include <Path.h>
#include <Log.h>
#include <PTM/SymbolUtils.h>
#include <PTM/OrbitModule.h>
#include <PTM/OrbitProcess.h>
#include <PTM/ProcessUtils.h>

#include <gtest/gtest.h>

#include <stdio.h>
#include <thread>
#include <proc/readproc.h>

using namespace std;

//ServerTimerManager* GServerTimerManager = new ServerTimerManager();

namespace UnitTest1
{
    class OrbitCoreTest : public testing::Test {
    protected:
        virtual void SetUp()
        {
            //GTcpServer = new TcpServer();
        }

        virtual void TearDown() 
        {
        }
    };

    TEST_F(OrbitCoreTest, BasicTest) 
    {
        ProcessList processList;

        processList.UpdateCpuTimes();
        processList.SortByCPU();

        for (const auto& p : processList.m_Processes)
        {
            p->EnumerateThreads();
            printf("%-15S, %6ld, %2s, %3zd, %*.3lf\n", 
                p->GetName().wstring().c_str(), 
                p->GetID(),
                p->GetIs64Bit() ? "64" : "32", 
                p->GetThreads().size(),
                6, p->GetCpuUsage()
            );
            
            /*for (const auto& orbthr : p->GetThreads())
            {
                auto& t = orbthr->m_Handle;
                printf("\t%d\n", t->tid);
            }*/
        }
        this_thread::sleep_for(chrono::seconds(1));
        processList.Refresh();

        for (const auto& p : processList.m_Processes)
        {
            SymUtils::ModuleMap_t modules = SymUtils::ListModules(p->GetID());
            for (const auto& m : modules)
            {
                printf("%s\n", m.second->m_FullName.c_str());
            }
            printf("\n");
        }
    }
}
