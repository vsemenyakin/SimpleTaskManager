#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

#include "utils/Future.h"

namespace TaskManagerImpl{ class ITask; }

class TaskManager
{
public:

    static TaskManager& TaskManager::Get()
    {
        static TaskManager Instance;
        return Instance;
    }
   
   template<typename FuncType, typename ... ArgTypes>
   auto Run(FuncType&& Func, ArgTypes&& ... Args)->
      Future<decltype(Func(Args ...))>;

   void ProcessFinishedTasksForCurrentThread();

private:

   ~TaskManager();

   void StartTasksThreadIfNeeded();
   void StartTasksLoop();

   void ProcessTasks();

   std::unique_ptr<std::thread> TasksThread;
   std::atomic<bool> IsTasksThreadTerminated;

   std::vector<std::shared_ptr<TaskManagerImpl::ITask>> FinishedTasks;
   std::mutex FinishedTasksMutex;
   
   std::queue<std::shared_ptr<TaskManagerImpl::ITask>> TasksQueue;
   std::mutex TasksQueueMutex;
};

#include "TasksManager.inl"
