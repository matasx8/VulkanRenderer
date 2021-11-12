#include "ThreadPool.h"

ThreadPool::ThreadPool()
	: m_Threads(), m_Mutex(), m_CV(), m_TerminatePool(false)
{
}

void ThreadPool::Create(size_t threadCount)
{
	size_t numThreads = 0;

	switch (threadCount)
	{
	case 0:
		numThreads = 10;
		// TODO: how should I pick this number?
		break;
	default:
		if (threadCount > std::thread::hardware_concurrency())
		{
			Debug::LogMsg("Provided thread count was more than your hardware supports. Using max supported instead\n");
				//threadCount, std::thread::hardware_concurrency());
			numThreads = std::thread::hardware_concurrency();
		}
		else
			numThreads = threadCount;
		break;
	}

	for (size_t i = 0; i < numThreads; i++)
	{
		//m_Threads.emplace_back(std::mem_fn(ThreadPool::RunThread));
	}
}

void ThreadPool::PushJob(Job job)
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_JobQueue.push(job);
}

size_t ThreadPool::GetFreeThreadCount()
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	return m_FreeThreads;
}

void ThreadPool::Destroy()
{
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_TerminatePool = true;
		m_CV.notify_all();
	}

	for (std::thread& thread : m_Threads)
	{
		thread.join();
	}
}

void ThreadPool::RunThread()
{
	Job job;
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		m_CV.wait(lock, [this]() {
			return !m_JobQueue.empty() || m_TerminatePool;
			});

		if (m_TerminatePool)
			return;

		job = m_JobQueue.front();
		m_JobQueue.pop();
	}

	job(); // function<void()> type

}
