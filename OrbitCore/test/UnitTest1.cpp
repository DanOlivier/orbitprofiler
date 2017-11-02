#include <Injection.h>
#include <ServerTimerManager.h>
#include <Path.h>
#include <TcpServer.h>
#include <Log.h>
#include <SymbolUtils.h>

#include <gtest/gtest.h>
//#include <shellapi.h>

#include <stdio.h>
#include <thread>

using namespace std;

ServerTimerManager* GTimerManager = new ServerTimerManager();

namespace UnitTest2
{
    class InjectionTest : public testing::Test {
    protected:
        virtual void SetUp()
        {
            GTcpServer = new TcpServer();
        }

        virtual void TearDown() 
        {
        }
    };

    TEST_F(InjectionTest, InjectNotepad) 
    {
        //ShellExecute(0, nullptr, L"C:\\WINDOWS\\system32\\notepad.exe", L"", 0, SW_HIDE);

        ProcessList processList;
        processList.SortByName();
        /*for (const auto& p : processList.m_Processes)
        {
            if(p->GetFullName().empty())
                continue;
            //printf("%S\n", p->GetFullName().wstring().c_str()); 
            if(!fs::exists(p->GetFullName()))
            {
                printf("%S, %S, %ld, %s, %zd\n", p->GetName().wstring().c_str(), 
                    p->GetFullName().wstring().c_str(), p->GetID(),
                    p->GetIs64Bit() ? "64" : "32", p->GetThreads().size());
            }
        }*/

        for (const auto& p : processList.m_Processes)
        {
            printf("%S, %S, %ld, %s, %zd\n", p->GetName().wstring().c_str(), 
                p->GetFullName().wstring().c_str(), p->GetID(),
                p->GetIs64Bit() ? "64" : "32", p->GetThreads().size());
            p->EnumerateThreads();
        }
    }
}
