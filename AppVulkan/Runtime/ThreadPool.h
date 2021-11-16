#pragma once
#include "NonCopyable.h"
#include "Debug.h"
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>

typedef std::function<void()> Job;

class ThreadPool : NonCopyable
{
public:
	ThreadPool();

	void Create(size_t threadCount = 0);
	void PushJob(Job job);

	size_t GetTotalThreadCount() const { return m_TotalThreads; }
	size_t GetFreeThreadCount();

	void Destroy();
private:

	void RunThread();

	std::vector<std::thread> m_Threads;
	std::mutex m_Mutex;
	std::condition_variable m_CV;

	size_t m_TotalThreads;
	size_t m_FreeThreads; 

	std::queue<Job> m_JobQueue;

	bool m_TerminatePool;
};

