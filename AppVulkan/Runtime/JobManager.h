#pragma once
#include "ThreadPool.h"
#include "Model.h"
#include <assert.h>
#include <functional>

namespace JobManager
{
	//typedef Job template<typename Func, typename... Args>
		/**
	template<typename Func, typename... Args>
	class Job
	{
	public:
		Job();

		void Run();

	private:

	};*/

	// Problem: can't tell which models are instanced. Instanced models are like even bigger containers of models.
	// To make it simple to the user I must make a Models data structure, that will seperate instanced and non instanced models
	// or something like that. Because now to fairly distribute work I'd have to iterate over the Models and find the instanced models
	// to treat them seperately. To save time now I'll make a temporary workaround that you have to provide the ModelHandles
	// that you know are instanced so the non-instanced jobs will ignore these models

	// Job function and its parameters
	//		problem: need to store them for threads to take
	//      solution: 
	//
	// Threads need to be always alive and be able to take any job they find and exec
	//		solution: infinite loop with CV
	//
	// Need to be able to signal when given job group is done
	//		potential solution: wait until all jobs are finished?
	// 
	template<typename... Args>
	static void CreateAndExecuteModelTransformationJobs(ThreadPool& pool, std::vector<Model>& Models, size_t numInstanced, std::function<void(Model&, Args...)> func)
	{
		if (pool.GetTotalThreadCount() == 0)
		{
			throw std::runtime_error("Failed to create model transformation jobs. Thread pool had no threads.");
		}

		// divide work
		//size_t totalWork = Models.size() + numInstanced;
		long actualWork = Models.size();
		size_t jobSize = actualWork / pool.GetTotalThreadCount();

		// add jobs
		while (actualWork > 0)
		{
			//size_t actualJobSize = actualWork - jobSize > 0 ? jobSize;
			pool.AddJob();
		}
		// wait

	}
};

