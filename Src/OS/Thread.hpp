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
#include <Core/TypeTraits.hpp>
#include <Core/Concepts.hpp>

#include <thread>
#include <future>
#include <atomic>
#include <semaphore>

struct Thread : std::jthread
{
    Thread() :
        std::jthread()
    {
    }

    Thread(Thread&& other) :
        std::jthread(Move(static_cast<std::jthread&&>(other)))
    {
    }

    template<class TF, class... TArgs >
        requires CInvocable<TF, TArgs...>
    Thread(TF&& f, TArgs&&... args ) :
        std::jthread(Forward<TF>(f), Forward<TArgs>(args)...)
    {
    }

    void Join()
    {
        std::jthread::join();
    }

    void Detach()
    {
        std::jthread::detach();
    }

    Thread& operator=(Thread&& other)
    {
        std::jthread::operator=(Move(other));
        return *this;
    }
};


template <typename T>
struct Atomic : std::atomic<T>
{
    Atomic(const T& a) :
        std::atomic<T>(a)
    {
    }

    auto operator=(const T& a) -> Atomic&
    {
        std::atomic<T>::operator=(a);
        return *this;
    }

    auto Load() const -> T
    {
        return std::atomic<T>::load();
    }

    operator T()
    {
        return Load();
    }
};


struct Mutex : std::mutex
{
    void Lock()
    {
        lock();
    }

    void Unlock()
    {
        unlock();
    }
};

template <typename... TMutexes>
using ScopedLock = std::scoped_lock<TMutexes...>;


template <typename T>
struct Future : std::future<T>
{
    Future() :
        std::future<T>()
    {
    }

    Future(Future&& other) :
        std::future<T>(Move(static_cast<std::future<T>&&>(other)))
    {
    }

    Future& operator=(Future&& other)
    {
        std::future<T>::operator=(Move(other));
        return *this;
    }

    T Get()
    {
        return std::future<T>::get();
    }
};


template <typename T>
struct Promise : std::promise<T>
{
    Promise(Promise&& other) :
        std::promise<T>(Move(static_cast<std::promise<T>&&>(other)))
    {
    }

    Promise& operator=(Promise&& other)
    {
        std::promise<T>::operator=(Move(other));
        return *this;
    }

    Future<T> GetFuture()
    {
        return Future<T>(Move(std::future<T>::get_future()));
    }
};


struct Semaphore : std::counting_semaphore<>
{
    Semaphore(I64 c) :
        std::counting_semaphore<>(c)
    {
    }

    void Acquire()
    {
        acquire();
    }

    void Release(I64 c = 1)
    {
        release(c);
    }
};


inline U32 GetLogicalCPUCount()
{
    return std::thread::hardware_concurrency();
}
