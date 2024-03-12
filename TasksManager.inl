#pragma once

namespace TaskManagerImpl
{
   class ITask
   {
   public:
       virtual const std::thread::id& GetClientThreadId() = 0;
       virtual void Perform() = 0;
       virtual void Finish() = 0;
   };

   template<typename FuncWrapperLambdaType>
   class Task : public ITask
   {
       using ResultType = decltype(std::declval<FuncWrapperLambdaType>()());
       
    public:
       Task(const std::thread::id InClientThreadId,
          FuncWrapperLambdaType&& FuncWrapperLambda)
          :
          ClientThreadId(InClientThreadId),
          FuncWrapperLambda(std::move(FuncWrapperLambda))
       {
       }
       
       const std::thread::id& GetClientThreadId() override
       {
           return ClientThreadId;
       }
       
       void Perform() override
       {
           Result = FuncWrapperLambda();
       }
       
       void Finish() override
       {
           ResultPromise.Set(std::move(Result));
       }
    
       Future<ResultType> GetResultFuture()
       {
           return ResultPromise.GetFuture();
       }

    private:
    
       const std::thread::id ClientThreadId;
       FuncWrapperLambdaType FuncWrapperLambda;
       
       ResultType Result;
       Promise<ResultType> ResultPromise;
   };
}

//----
   
template<typename FuncType, typename ... ArgTypes>
auto TaskManager::Run(FuncType&& Func, ArgTypes&& ... Args)->
  Future<decltype(Func(Args ...))>
{
   //Wrapper Lambda
   auto FuncWrapperLambda = std::bind(
	   std::forward<FuncType>(Func),
	   std::forward<ArgTypes>(Args) ...);

	using FuncWrapperLambdaType = decltype(FuncWrapperLambda);

	const std::thread::id ClientThreadId = std::this_thread::get_id();

	auto NewTask = std::make_shared<TaskManagerImpl::Task<FuncWrapperLambdaType>>(
		ClientThreadId, std::move(FuncWrapperLambda));
  
	TasksQueue.push(NewTask);
	
	StartTasksThreadIfNeeded();
	
	return NewTask->GetResultFuture();
}

void TaskManager::ProcessFinishedTasksForCurrentThread()
{
   std::lock_guard<std::mutex> Guard(FinishedTasksMutex);
   
   size_t Index = 0;
   while (Index < FinishedTasks.size())
   {
	   const std::shared_ptr<TaskManagerImpl::ITask>& Task = FinishedTasks[Index];
	   
	   if (Task->GetClientThreadId() != std::this_thread::get_id())
	   {
		  ++Index;
		  continue;
	   }
	   
	   Task->Finish();
	   
	   std::swap(FinishedTasks[Index], FinishedTasks.back());
	   FinishedTasks.pop_back();
   }
}

TaskManager::~TaskManager()
{
   if (TasksThread)
   {
	   IsTasksThreadTerminated = true;
	   TasksThread->join();
   }
}

void TaskManager::StartTasksThreadIfNeeded()
{
   if (!TasksThread)
   {
	  TasksThread = std::make_unique<std::thread>(
		  &TaskManager::StartTasksLoop, this);           
   }
}

void TaskManager::StartTasksLoop()
{
   while (!IsTasksThreadTerminated)
   {
	   ProcessTasks();
   }
}

void TaskManager::ProcessTasks()
{
   while (TasksQueue.size() > 0)
   {
	   std::shared_ptr<TaskManagerImpl::ITask> CurrentTask;
	   
	   {
		   std::lock_guard<std::mutex> Guard(TasksQueueMutex);
		   
		   CurrentTask = TasksQueue.front();
		   TasksQueue.pop();
	   }
	   
	   CurrentTask->Perform();
	   
	   {
		   std::lock_guard<std::mutex> Guard(FinishedTasksMutex);
		   
		   FinishedTasks.push_back(CurrentTask);
	   }
   }
}
