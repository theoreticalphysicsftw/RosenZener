// MIT License
// 
// Copyright (c) 2024 Mihail Mladenov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <Core/Primitives.hpp>
#include <Core/ExternAlias.hpp>
#include <Core/Deque.hpp>
#include <OS/Thread.hpp>


template <typename T>
struct TaskResult
{
    auto Retrieve() -> T;
    Future<T> future;
};


template <typename TTask = Function<Void()>>
class ThreadPool
{
public:
    using Task = TTask;

    static auto Init(U32 numberOfThreads = GetLogicalCPUCount()) -> Void;

    template <typename TFunc, typename... TArgs>
    static auto AddTask(TFunc f, TArgs... args) -> TaskResult<InvokeResult<TFunc, TArgs...>>;

    static auto GetMaxTasks() -> U32;
    static auto ShutDown() -> Void;

private:
    inline static Atomic<Bool> keepRunning = true;
    inline static Mutex queueMutex;
    inline static Deque<Task> queue;
    inline static Array<Thread> threads;
    inline static Semaphore availableTasks = Semaphore(0);
};


template<typename T>
inline auto TaskResult<T>::Retrieve() -> T
{
    return future.Get();
}


template<typename TTask>
inline auto ThreadPool<TTask>::Init(U32 numberOfThreads) -> Void
{
    for (auto i = 0u; i < numberOfThreads; ++i)
    {
        threads.EmplaceBack
        (
            [&]()
            {
                while (keepRunning)
                {
                    availableTasks.acquire();
                    if (!keepRunning)
                    {
                        break;
                    }
                    queueMutex.Lock();
                    auto front = queue.front();
                    queue.PopFront();
                    queueMutex.Unlock();

                    Invoke<Task>(Move(front));
                }
            }
        );
    }
}


template<typename TTask>
inline auto ThreadPool<TTask>::GetMaxTasks() -> U32
{
    return threads.GetSize();
}

template<typename TTask>
inline auto ThreadPool<TTask>::ShutDown() -> Void
{
    keepRunning = false;
    availableTasks.Release(threads.GetSize());
}


template<typename TTask>
template<typename TFunc, typename ...TArgs>
inline auto ThreadPool<TTask>::AddTask(TFunc f, TArgs... args) -> TaskResult<InvokeResult<TFunc, TArgs...>>
{
    using Result = InvokeResult<TFunc, TArgs...>;
    
    TaskResult<Result> result;

    queueMutex.Lock();
    auto promise = RefPtr<Promise<Result>>(new Promise<Result>);
    result.future = promise->GetFuture();
    queue.EmplaceBack
    (
        [promiseL = promise, fL = Move(f), ... argsL = Move(args)]()
        {
            if constexpr (IsSameType<Result, Void>)
            {
                fL(argsL...);
                promiseL->set_value();
            }
            else
            {
                promiseL->set_value(fL(argsL...));
            }
        }
    );
    queueMutex.Unlock();
    availableTasks.Release();
    return result;
}


using GThreadPool = ThreadPool<>;
