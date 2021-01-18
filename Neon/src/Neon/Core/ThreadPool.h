#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Neon
{
	class ThreadPool
	{
	public:
		ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());

		~ThreadPool();

		template<class F, class... Args>
		auto QueueTask(F&& f, Args&&... args)
		{
			auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...));

			std::future<decltype(f(args...))> res = task->get_future();
			{
				std::lock_guard<std::mutex> guard(m_Mutex);

				if (m_Stop)
				{
					throw std::runtime_error("Queuing task on stopped ThreadPool!");
				}

				m_Tasks.emplace([task]() { (*task)(); });
				m_Condition.notify_one();
			}

			return res;
		}

	private:
		void ThreadWork();

	private:
		std::mutex m_Mutex{};
		std::vector<std::thread> m_Workers{};
		std::queue<std::function<void()>> m_Tasks{};
		std::condition_variable m_Condition{};

		bool m_Stop = false;
	};
} // namespace Neon
