#include "ThreadPool.h"
#include <functional>

ThreadPool::ThreadPool()
{
	const int threadCount = std::thread::hardware_concurrency() - 2;

	for (int i = 0; i < threadCount; i++)
	{
		threads.emplace_back([this] {
			while (true)
			{
				std::function<void()> job;
				{
					std::unique_lock<std::mutex> lock{ mutex };
					threadWait.wait(lock, [this] { return !jobs.empty() || stop; });

					if (stop)
						return;

					job = std::move(jobs.front());
					jobs.pop();
				}
				job();
			}
		});
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::lock_guard lock{ mutex };
		stop = true;
	}

	threadWait.notify_all();

	for (int i = 0; i < threads.size(); i++)
	{
		if (threads[i].joinable())
		{
			threads[i].join();
		}
	}
}

int ThreadPool::GetThreadCount() const
{
	return threads.size();
}