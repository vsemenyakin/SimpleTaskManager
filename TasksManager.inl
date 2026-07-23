#pragma once
   
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
		std::move(FuncWrapperLambda));

	std::shared_ptr<TaskManagerImpl::Thread> Thread = FindOrCreateTheFreestThread();
	Thread->AddTask(NewTask);	
	Thread->StartIfNeeded();
	
	return NewTask->GetResultFuture();
}

std::shared_ptr<TaskManagerImpl::Thread> TaskManager::FindOrCreateTheFreestThread()
{
	if (Threads.size() == 0)
	{
		auto FirstThread = std::make_shared<TaskManagerImpl::Thread>();
		Threads.push_back(FirstThread);

		return FirstThread;
	}

	//Try find free thread
	for (std::shared_ptr<TaskManagerImpl::Thread> Thread : Threads)
	{
		if (Thread->GetTasksInQueueCount() == 0)
		{
			return Thread;
		}
	}

	//If all threads are busy, try create new one
	if (Threads.size() < MaxThreads)
	{
		auto NewThread = std::make_shared<TaskManagerImpl::Thread>();
		Threads.push_back(NewThread);

		return NewThread;
	}

	//If there are no free threads - find the freest one
	auto ThreadsIterator = Threads.begin();

	std::shared_ptr<TaskManagerImpl::Thread> SelectedThread = *ThreadsIterator;
	for (; ThreadsIterator != Threads.end(); ++ThreadsIterator)
	{
		std::shared_ptr<TaskManagerImpl::Thread> CurrentThread = *ThreadsIterator;
		if (CurrentThread->GetTasksInQueueCount() < SelectedThread->GetTasksInQueueCount())
		{
			SelectedThread = CurrentThread;
		}
	}

	return SelectedThread;
}

void TaskManager::ProcessFinishedTasks()
{
	std::vector<std::shared_ptr<TaskManagerImpl::ITask>> FinishedTasks;

	for (std::shared_ptr<TaskManagerImpl::Thread> Thread : Threads)
	{
		Thread->GetAndClearFinishedTasks(FinishedTasks);
	}

	for (const std::shared_ptr<TaskManagerImpl::ITask>& FinishedTask : FinishedTasks)
	{
		FinishedTask->Finish();
	}
}

TaskManager::~TaskManager()
{
	for (const std::shared_ptr<TaskManagerImpl::Thread>& Thread : Threads)
	{
		Thread->Close();
	}
}
