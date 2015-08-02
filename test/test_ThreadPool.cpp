/**
Copyright (c) 2013, Riccardo Ressi
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Riccardo Ressi nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ThreadPool.h"
#include "test_Utils.h"

#include "Trace.h"
#include "Mutex.h"

#include <iostream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------

namespace {

class TestTask
        :
                public ITask
{

    int m_id;
    Mutex &m_mutex;
    int &m_instance_counter;
    int &m_execution_counter;
    int m_step;

    bool m_trace;

public:

    TestTask(int id,
             Mutex &mutex,
             int &instance_counter,
             int &execution_counter)
            :
            m_id(id),
            m_mutex(mutex),
            m_instance_counter(instance_counter),
            m_execution_counter(execution_counter),
            m_step(0),
            m_trace((id % 100000) == 0)
    {
        {
            Locker<Mutex> locker(m_mutex);
            ++m_instance_counter;
        }

        trace_opt("created");
    }

    virtual
    ~TestTask()
    {
        {
            Locker<Mutex> locker(m_mutex);
            TEST_CHECK(m_instance_counter > 0);
            --m_instance_counter;
        }

        trace_opt("destroyed");

        TEST_CHECK(m_step == 1);
        ++m_step;
    }

    virtual void
    execute()
    {
        {
            Locker<Mutex> locker(m_mutex);
            ++m_execution_counter;
        }

        trace_opt("executed");

        TEST_CHECK(m_step == 0);
        ++m_step;
    }

    virtual void
    trace_opt(std::string message) const
    {
        if (m_trace)
        {
            trace(m_id, message);
        }
    }

};

} // anonymous namespace

// -----------------------------------------------------------------------------

void
test_ThreadPool()
{
    const int NUM_THREADS = 16;
    const int NUM_TASKS = 1000000;
    const int QUEUE_CAPACITY = 100;

    std::unique_ptr<IThreadPool> pool(
            IThreadPool::create(NUM_THREADS, QUEUE_CAPACITY));

    Mutex mutex;
    int num_tasks_in = NUM_TASKS;
    int num_tasks_out = NUM_TASKS;
    int instance_counter = 0;
    int execution_counter = 0;

    while (num_tasks_in > 0
            || num_tasks_out > 0)
    {

        if (num_tasks_in > 0)
        {
            int id = NUM_TASKS - num_tasks_in;

            Task task(new TestTask(id, mutex, instance_counter,
                                   execution_counter));

            std::size_t num = pool->push(task);
            if (num > 0)
            {
                --num_tasks_in;
            }
            else
            {
                //sched_yield();
            }
        }

        if (num_tasks_out > 0)
        {
            std::shared_ptr<TestTask> task;
            std::size_t num = pool->popT(task, num_tasks_in > 0);
            if (num > 0)
            {
                task->trace_opt("collected");
                --num_tasks_out;
            }
            else
            {
                //sched_yield();
            }
        }

    }

    pool->join();

    TEST_CHECK(0 == instance_counter);
    TEST_CHECK(NUM_TASKS == execution_counter);
}

// -----------------------------------------------------------------------------
