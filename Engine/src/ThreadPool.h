#pragma once
#include <thread>
#include <future>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) noexcept = delete;

	template <class T>
	std::future<T> AddTask(std::function<T()> job);
	
	int GetThreadCount() const;

private:
	std::vector<std::jthread> threads;
	std::queue<std::function<void()>> jobs;
	std::condition_variable threadWait;
	std::mutex mutex;
	bool stop = false;
};


template <class T>
inline std::future<T> ThreadPool::AddTask(std::function<T()> job)
{
	auto task = std::make_shared<std::packaged_task<T()>>(job);
	std::future<T> future = task->get_future();

	{
		std::lock_guard<std::mutex> lock{ mutex };
		jobs.emplace([task] { (*task)(); });
	}

	threadWait.notify_one();

	return future;
}