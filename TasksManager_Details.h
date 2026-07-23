#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

#include "utils/Future.h"

namespace TaskManagerImpl
{
	class ITask
	{
	public:
		virtual void Perform() = 0;
		virtual void Finish() = 0;
	};

	template<typename FuncWrapperLambdaType>
	class Task : public ITask
	{
		using ResultType = decltype(std::declval<FuncWrapperLambdaType>()());

	public:
		Task(FuncWrapperLambdaType&& FuncWrapperLambda)
			:
			FuncWrapperLambda(std::move(FuncWrapperLambda))
		{
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
		FuncWrapperLambdaType FuncWrapperLambda;

		ResultType Result;
		Promise<ResultType> ResultPromise;
	};

	//----

	class Thread
	{
	public:
		void StartIfNeeded()
		{
			if (Thread)
			{
				//Try wake up thread
				WorkingConditionVariable.notify_one();
				return;
			}

			Thread = std::make_unique<std::thread>(
				&Thread::StartTasksLoop, this);
		}

		void AddTask(const std::shared_ptr<ITask>& Task)
		{
			std::lock_guard<std::mutex> Guard(TasksQueueMutex);

			TasksQueue.push(Task);
		}

		size_t GetTasksInQueueCount() const
		{
			std::lock_guard<std::mutex> Guard(TasksQueueMutex);

			return TasksQueue.size();
		}

		void Close()
		{
			IsThreadStopped = true;
			Thread->join();
		}

		void GetAndClearFinishedTasks(std::vector<std::shared_ptr<ITask>>& OutFinishedTasks)
		{
			std::lock_guard<std::mutex> Guard(FinishedTasksMutex);

			for (const std::shared_ptr<ITask>& FinishedTask : FinishedTasks)
			{
				OutFinishedTasks.push_back(FinishedTask);
			}

			FinishedTasks.clear();
		}

	private:

		//Worker thread
		void StartTasksLoop()
		{
			while (!IsThreadStopped)
			{
				ProcessTasks();
			}
		}

		//Worker thread
		void ProcessTasks()
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

			{
				std::unique_lock<std::mutex> Guard(TasksQueueMutex);
				WorkingConditionVariable.wait(Guard, [this]()
					{
						return this->TasksQueue.size() > 0;
					});
			}
		}

		std::unique_ptr<std::thread> Thread;
		std::atomic<bool> IsThreadStopped;

		std::queue<std::shared_ptr<ITask>> TasksQueue;
		mutable std::mutex TasksQueueMutex;

		std::vector<std::shared_ptr<ITask>> FinishedTasks;
		std::mutex FinishedTasksMutex;

		std::condition_variable WorkingConditionVariable;
	};
}
