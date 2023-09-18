#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <atomic>

// future�̏�Ԃ��m�F����
template<typename R> bool is_readyy(std::future<R> const& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

// --------------------------------------------------------------------------------------------------------------------------------------
//
// Sync Objects
//
//---------------------------------------------------------------------------------------------------------------------------------------
//
// �V�O�i��
//
class Signal
{
public:
	inline void NotifyOne() { cv.notify_one(); }; // �ǂꂩ��ɒʒm����
	inline void NotifyAll() { cv.notify_all(); };

	inline void Wait() { std::unique_lock<std::mutex> lk(this->mtx); this->cv.wait(lk); };
	inline void Wait(bool(*pPred)()) { std::unique_lock<std::mutex> lk(this->mtx); this->cv.wait(lk, pPred); }; //pPred��true���Anotify������܂őҋ@
	template<class Functor> 
	inline void Wait(Functor fn) { std::unique_lock<std::mutex> lk(this->mtx); this->cv.wait(lk, fn); };
	
private:
	std::mutex mtx;
	std::condition_variable cv;
};

//
// �Z�}�t�H
//
class Semaphore
{
public:
	Semaphore(int val, int max) : maxVal(max), currVal(val) {};

	inline void P() { Wait();  } // �g�p
	inline void V() { Signal(); } // ���
	void Wait();
	void Signal();
private:
	unsigned short currVal, maxVal;
	std::mutex mtx;
	std::condition_variable cv;
};



//--------------------------------------------------
//
// ThreadPool
//
//--------------------------------------------------
//
// �^�X�N�L���[
// 
using Task = std::function<void()>; // Task�͊֐��I�u�W�F�N�g�Ƃ��Ĉ���
class TaskQueue
{
public:
	template<class T>
	void AddTask(std::shared_ptr<T>& pTask);
	bool TryPopTask(Task& task);

	inline bool IsQueueEmpty() const { std::unique_lock<std::mutex> lock(mutex); return queue.empty(); } // �L���[���󂩂ǂ��� ( ���b�N����� )
	inline int GetNumActivaTasks() const { return activeTasks; }

	// �^�X�N�����������Ƃ��ɌĂ�
	inline void OnTaskComplete() { --activeTasks; }

private:
	std::atomic<int> activeTasks = 0; // ���ݎ��s���̃^�X�N��
	mutable std::mutex mutex; // �r������
	std::queue<Task> queue; // �^�X�N��ۑ�����L���[
};
template<class T>
inline void TaskQueue::AddTask(std::shared_ptr<T>& pTask)
{
	std::unique_lock<std::mutex> lock(mutex);
	queue.emplace([=]() { (*pTask)(); }); // �L���[�Ƀ^�X�N�����s���Ēǉ�
	++activeTasks;
}

//
// �X���b�h�v�[��
//
class ThreadPool
{
public:
	const static size_t sHardwareThreadCount;

	void Initialize(size_t numWorkers, const std::string& threadPoolName);
	void Destroy();

	inline int GetNumActiveTasks() const { return IsExiting() ? 0 : mTaskQueue.GetNumActivaTasks(); }; // 
	inline size_t GetThreadPoolSize() const { return mWorkers.size(); }
	
	inline bool IsExiting() const { return mbStopWorkers.load(); }

	template<class T>
	auto AddTask(T task) -> std::future<decltype(task()) > ; // task()�̕Ԃ�l�̌^��Future��Ԃ�

private:
	void Execute();

	Signal						mSignal;
	std::atomic<bool>			mbStopWorkers; // �X���b�h���~���邩�ǂ���
	TaskQueue					mTaskQueue;
	std::vector<std::thread>	mWorkers;
	std::string					mThreadPoolName;  // �X���b�h�v�[���̖��O ( 1, 2, 3 ) 
};

template<class T>
auto ThreadPool::AddTask(T task)-> std::future<decltype(task())>
{
	using task_return_t = decltype(task()); // task()�̕Ԃ�l�̌^

	auto pTask = std::make_shared<std::packaged_task<task_return_t()>>(std::move(task));
	mTaskQueue.AddTask(pTask);

	mSignal.NotifyOne(); // �ǂꂩ��ɒʒm����

	return pTask->get_future();
}

// --------------------------------------------------------------------------------------------------------------------------------------
//
// Buffered Container
//
//---------------------------------------------------------------------------------------------------------------------------------------
template<class TContainer, class TItem>
class BufferedContainer
{
public:
	TContainer& GetBackContainer() { return mBufferPool[(iBuffer + 1) % 2]; };
	const TContainer& GetBackContainer() const { return mBufferPool[(iBuffer + 1) % 2]; };

	void AddItem(TItem& item)
	{
		std::unique_lock<std::mutex> lk(mMtx);
		mBufferPool[iBuffer].emplace(std::forward<TItem&&>(item));
	}

	void AddItem(const TItem& item)
	{
		std::unique_lock<std::mutex> lk(mMtx);
		mBufferPool[iBuffer].emplace(item);
	}

	void SwapBuffers()
	{
		std::unique_lock<std::mutex> lk(mMtx);
		iBuffer ^= 1;
	}

	bool IsEmpty()
	{
		std::unique_lock<std::mutex> lk(mMtx);
		return mBufferPool[iBuffer].empty();
	}

private:
	mutable std::mutex mMtx;
	std::array<TContainer, 2> mBufferPool;
	int iBuffer = 0; // 0 or 1
};

// ����L���[
template<class T>
class ConcurrentQueue
{
public:
	ConcurrentQueue(void (*pfnPRocess)(T&)) : mpfnProcess(pfnPRocess) {}

	void Enqueue(const T& item);
	void Enqueue(const T&& item);
	T Dequeue();

	void ProcessItems();

private:
	mutable std::mutex mMtx;
	std::queue<T> mQueue;
	void(*mpfnProcess)(T&); // �֐��|�C���^
};

template<class T>
inline void ConcurrentQueue<T>::Enqueue(const T& item)
{
	std::lock_guard<std::mutex> lk(mMtx);
	mQueue.push(item);
}

template<class T>
inline void ConcurrentQueue<T>::Enqueue(const T&& item)
{
	std::lock_guard<std::mutex> lk(mMtx);
	mQueue.push(std::move(item));
}

template<class T>
inline T ConcurrentQueue<T>::Dequeue()
{
	std::lock_guard<std::mutex> lk(mMtx);
	T item = mQueue.front();
	mQueue.pop();
	return T;
}

template<class T>
inline void ConcurrentQueue<T>::ProcessItems()
{
	if (!mpfnProcess)
		return;

	std::unique_lock<std::mutex> lk(mMtx);
	do
	{
		T&& item = std::move(mQueue.front());
		mQueue.pop();
		mpfnProcess(item);
	} while (!mQueue.empty());
}
