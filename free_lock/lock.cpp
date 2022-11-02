#include "lock.h"
namespace swd
{
	// IdentityNumber

	std::atomic<size_t> IdentityNumber::SM_NumberMaker;

	IdentityNumber::IdentityNumber()
		: m_number(SM_NumberMaker++)
	{
	}

	IdentityNumber::~IdentityNumber()
	{
		//不用归还，64位的数据不考虑能被用完。
	}

	IdentityNumber::IdentityNumber(IdentityNumber &&aim) noexcept
	{
		std::swap(aim.m_number, m_number);
	}

	size_t IdentityNumber::GetNumber() const
	{
		return m_number;
	}

	void IdentityNumber::operator=(IdentityNumber &&aim) noexcept
	{
		std::swap(aim.m_number, m_number);
	}

	bool IdentityNumber::operator==(const IdentityNumber &aim) const
	{
		return m_number == aim.m_number;
	}

	bool IdentityNumber::operator<(const IdentityNumber &aim) const
	{
		return m_number < aim.m_number;
	}

	bool IdentityNumber::operator>(const IdentityNumber &aim) const
	{
		return m_number > aim.m_number;
	}

	// mutex

	mutex::mutex()
		: m_V(false)
	{

	}

	mutex::mutex(mutex &&aim) noexcept
	{
		m_V.exchange(aim.m_V.exchange(m_V));
	}

	mutex &mutex::operator=(mutex &&aim) noexcept
	{
		m_V.exchange(aim.m_V.exchange(m_V));
		return *this;
	}

	void mutex::lock()
	{
		bool te = false;
		while (!m_V.compare_exchange_weak(te, true))
		{
			te = false;
			_WAIT_TIME(1, ms);
		}
#ifdef _DEBUG
		m_NowHoldThread = std::this_thread::get_id();
#endif // _DEBUG
	}

	bool mutex::try_lock()
	{
		bool te = false, result;
#ifdef _DEBUG
		auto te_HoldThread = m_NowHoldThread.load();
#endif // _DEBUG
		result = m_V.compare_exchange_weak(te, true);
#ifdef _DEBUG
		m_NowHoldThread.compare_exchange_strong(te_HoldThread,std::thread::id());
#endif // _DEBUG
		return result;
	}

	void mutex::unlock()
	{
#ifdef _DEBUG
		m_NowHoldThread = std::thread::id();
#endif // _DEBUG
		m_V.store(false);
	}

	// Lock_Memger

	thread_local Lock_Memger Lock_Memger::TM_LM;

	Lock_Memger::~Lock_Memger()
	{
		for (auto &var : m_lock_pool)
		{
			var->unlock();
		}
	}

	void Lock_Memger::Push(lock_base *aim)
	{
		auto InsertResult = m_lock_pool.insert(aim);
		if (InsertResult.second)
		{
			++InsertResult.first;//从下一个开始
			for (auto begin = InsertResult.first; begin != m_lock_pool.end(); ++begin)
			{
				begin.operator*()->unlock(); //解锁
			}
			aim->lock();
			for (auto begin = InsertResult.first; begin != m_lock_pool.end(); ++begin)
			{
				begin.operator*()->lock(); //加锁
			}
		}
		else
		{
			//同一线程将一个锁加多次应该强制修改
#ifdef SWD_LOCK_TEST //是否检测有无因调用导致的死锁
			assert(false); //你的线程有死锁。
#endif					   // SWD_LOCK_TEST
		}
	}

	void Lock_Memger::Pop(lock_base *aim)
	{
		auto FindResult = m_lock_pool.find(aim);
		if (FindResult == m_lock_pool.end())
		{
			//assert(false); //解锁一个未加锁的锁。
		}
		else
		{
			FindResult.operator*()->unlock();
			m_lock_pool.erase(FindResult);
		}
	}

	void Lock_Memger::Pause()
	{
		for (auto &var : m_lock_pool)
		{
			var->unlock();
		}
	}

	void Lock_Memger::Regain()
	{
		for (auto &var : m_lock_pool)
		{
			var->lock();
		}
	}

	// Thread

	thread::~thread()
	{
	}

	void thread::join()
	{
		if (std::thread::joinable())
		{
#ifdef SWD_LOCK_TEST		   //是否检测有无因调用导致的死锁
			_WAIT_TIME(20, s); //因为是检测，所以无所谓时间有多久，就是看看子线程能不能结束，20秒都不结束，建议看看是不是死锁了。
			assert(!std::thread::joinable());
#endif // _DEBUG
			Lock_Memger::TM_LM.Pause();
			std::thread::join();
			Lock_Memger::TM_LM.Regain();
		}
		else
		{
			std::thread::join();
		}
	}
}
