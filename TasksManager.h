#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

#include "utils/Future.h"

#include "TasksManager_Details.h"

class TaskManager
{
public:
    constexpr static size_t MaxThreads = 4;

    static TaskManager& TaskManager::Get()
    {
        static TaskManager Instance;
        return Instance;
    }
   
    //Should be called from main thread only
    //Returned future is always filled from main thread
    template<typename FuncType, typename ... ArgTypes>
    auto Run(FuncType&& Func, ArgTypes&& ... Args)->
       Future<decltype(Func(Args ...))>;
    
    //Should be called from main thread
    void ProcessFinishedTasks();
    
private:
    std::shared_ptr<TaskManagerImpl::Thread> FindOrCreateTheFreestThread();

    ~TaskManager();
   
    std::vector<std::shared_ptr<TaskManagerImpl::Thread>> Threads;
};

#include "TasksManager.inl"
