#define NOMINMAX

#include "Multithreading.h"

#include <algorithm>
#include <string>

#define RUN_THREADPOOL_UNIT_TEST 0

// �����n�ŃT�|�[�g�����X���b�h���s���̎擾
const size_t ThreadPool::sHardwareThreadCount = std::thread::hardware_concurrency();

// �X���b�h�e�X�g�֐� 16�X���b�h���쐬���A�����_���ȕ����𐶐��A���Z����
static void RUN_THREAD_POOL_UNIT_TEST()
{
	ThreadPool p;
	p.Initialize(ThreadPool::sHardwareThreadCount, "TEST POOL");

	constexpr long long sz = 40000000;
	auto sumRnd = [&]()
	{
		std::vector<long long> nums(sz, 0);
		for (int i = 0; i < sz; ++i)
		{
			// nums[i] = MathUtil::RandI(0, 5000);
		}
		unsigned long long result = 0;
		for (int i = 0; i < sz; ++i)
		{
			if (nums[i] > 3000)
				result += nums[i];
		}
		return result;
	};
	auto sum = [&]()
	{
		std::vector<long long> nums(sz, 0);
		for (int i = 0; i < sz; ++i)
		{
			// nums[i] = MathUtil::RandI(0, 5000);
		}
		unsigned long long result = 0;
		for (int i = 0; i < sz; ++i)
		{
			result += nums[i];
		}
		return result;
	};

	constexpr int threadCount = 16;
	std::future<unsigned long long> futures[threadCount] =
	{
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),
		p.AddTask(sumRnd),

		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
		p.AddTask(sum),
	};

	std::vector<unsigned long long> results;
	unsigned long long total = 0;
	std::for_each(std::begin(futures), std::end(futures), [&](decltype(futures[0]) f) // �ϐ��̃L���v�`��
	{
			results.push_back(f.get()); // get()��future�̌��ʂ��擾�ł���܂őҋ@
			total += results.back();
	});

	std::string strResult = "total (" + std::to_string(total) + ") = ";
	for (int i = 0; i < threadCount; ++i)
	{
		strResult += "(" + std::to_string(results[i]) + ")" + (i == threadCount - 1 ? "" : " + ");
	}
}

void Semaphore::Wait()
{
	std::unique_lock<std::mutex> lk(mtx);
	cv.wait(lk, [&] {return currVal > 0; });
	--currVal;
	return;
}

void Semaphore::Signal()
{
	std::unique_lock<std::mutex> lk(mtx);
	currVal = std::min<unsigned short>(currVal + 1u, maxVal);
	cv.notify_one();
}

void ThreadPool::Initialize(size_t numThreads, const std::string& ThreadPoolName)
{
	mThreadPoolName = ThreadPoolName;
	mbStopWorkers.store(false);
	for (auto i = 0u; i < numThreads; ++i)
	{
		mWorkers.emplace_back(std::thread(&ThreadPool::Execute, this));
	}

#if RUN_THREADPOOL_UNIT_TEST
	RUN_THREAD_POOL_UNIT_TEST();
#endif
}

void ThreadPool::Destroy()
{
	mbStopWorkers.store(false);

	mSignal.NotifyAll();

	for (auto& worker : mWorkers)
	{
		worker.join();
	}
}

void ThreadPool::Execute()
{
	Task task;

	// �I���ʒm������܂Ń^�X�N�̏������s��
	while (!mbStopWorkers)
	{
		mSignal.Wait([&] { return mbStopWorkers || !mTaskQueue.IsQueueEmpty(); });

		if (mbStopWorkers)
			break;

		// �^�X�N�̃`�F�b�N�Ǝ��o��
		if (!mTaskQueue.TryPopTask(task))
		{
			continue;
		}

		// �^�X�N�̎��s
		task();
		mTaskQueue.OnTaskComplete();
	}
}

bool TaskQueue::TryPopTask(Task& task)
{
	std::lock_guard<std::mutex> lk(mutex);

	if (queue.empty())
		return false;

	task = std::move(queue.front());
	queue.pop();

	return true;	
}