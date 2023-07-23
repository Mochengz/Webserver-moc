// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "logging.h"
#include "thread.h"
#include <string>
#include <unistd.h>
#include <vector>
#include <memory>
#include <iostream>
using namespace std;

void ThreadFunc()
{
    for (int i = 0; i < 100000; ++i)
    {
        LOG << i;
    }
}

void TypeTest()
{
    // 13 lines
    cout << "----------type test-----------" << endl;
    LOG << 0;
    LOG << 1234567890123;
    LOG << 1.0f;
    LOG << 3.1415926;
    LOG << (short) 1;
    LOG << (long long) 1;
    LOG << (unsigned int) 1;
    LOG << (unsigned long) 1;
    LOG << (long double) 1.6555556;
    LOG << (unsigned long long) 1;
    LOG << 'c';
    LOG << "abcdefg";
    LOG << string("This is a string");
}

void StressingSingleThread()
{
    // 100000 lines
    cout << "----------stressing test single thread-----------" << endl;
    for (int i = 0; i < 100000; ++i)
    {
        LOG << i;
    }
}

void StressingMultiThreads(int threadNum = 4)
{
    // threadNum * 100000 lines
    cout << "----------stressing test multi thread-----------" << endl;
    vector<shared_ptr<Thread>> vsp;
    for (int i = 0; i < threadNum; ++i)
    {
        shared_ptr<Thread> tmp(new Thread(ThreadFunc, "testFunc"));
        vsp.push_back(tmp);
    }
    for (int i = 0; i < threadNum; ++i)
    {
        vsp[i]->Start();
    }
    sleep(3);
}

void Other()
{
    // 1 line
    cout << "----------other test-----------" << endl;
    LOG << "fddsa" << 'c' << 0 << 3.666 << string("This is a string");
}


int main()
{
    // 共500014行
    TypeTest();
    sleep(3);

    StressingSingleThread();
    sleep(3);

    Other();
    sleep(3);

    StressingMultiThreads();
    sleep(3);
    return 0;
}