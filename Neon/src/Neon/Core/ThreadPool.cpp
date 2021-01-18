#include "neopch.h"

#include "ThreadPool.h"

namespace Neon
{
	ThreadPool::ThreadPool(uint32 threadCount /*= std::thread::hardware_concurrency()*/)
	{
		for (uint32 i = 0; i < threadCount; i++)
		{
			m_Workers.emplace_back(&ThreadPool::ThreadWork, this);
		}
	}

	ThreadPool::~ThreadPool()
	{
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			m_Stop = true;
			m_Condition.notify_all();
		}

		for (std::thread& worker : m_Workers)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
	}

	void ThreadPool::ThreadWork()
	{
		while (true)
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(m_Mutex);

				m_Condition.wait(lock, [this]() { return m_Stop || !m_Tasks.empty(); });
				if (m_Stop && m_Tasks.empty())
				{
					return;
				}
				task = std::move(m_Tasks.front());
				m_Tasks.pop();
			}
			task();
		}
	}
} // namespace Neon
