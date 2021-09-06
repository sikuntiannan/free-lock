#include"Lock.h"
namespace swd
{
	RW_Lock::RW_Lock ()
		:m_ReadNumber (0), m_Write (false)
	{
	}

	RW_Lock::~RW_Lock ()
	{
		{
			//设置写状态，保证写结束。
			bool te = false;
			while (!m_Write.compare_exchange_weak (te, true))
			{
				te = false;
				_WAIT_TIME (1, ms);
			}
		}
		{
			//保证读结束。
			while (m_ReadNumber.load ())
			{
				_WAIT_TIME (1, ms);
			}
		}
	}

	/*
	锁是抢占的，双方具有公平性，不会应为读连续导致写长时间挂起。
	*/

	void RW_Lock::Read_Lock ()
	{
		while (!m_Write.load ())//等待Write_Lock的解锁。
		{
			_WAIT_TIME (1, ms);
		}
		//有可能在这里（这句话的位置），完成了Write_Lock的调用，所以下面再次等待写结束。
		++m_ReadNumber;//阻止Write_Lock返回
		while (!m_Write.load ())//等待Write_Lock的解锁。
		{
			_WAIT_TIME (1, ms);
		}
	}

	void RW_Lock::Write_Lock ()
	{
		bool te = false;
		while (!m_Write.compare_exchange_weak (te, true))//等待Write_Lock的解锁。
		{
			_WAIT_TIME (1, ms);
		}
		while (m_ReadNumber.load ())//等待Read_Lock的解锁。
		{
			_WAIT_TIME (1, ms);
		}
	}

	ErrorCode RW_Lock::Try_Read_Lock ()
	{
		if (!m_Write.load ())
		{
			return ErrorCode::FAIL;
		}
		++m_ReadNumber;
		if (!m_Write.load ())
		{
			--m_ReadNumber;
			return ErrorCode::FAIL;
		}
		return ErrorCode::SUCCESS;
	}

	ErrorCode RW_Lock::Try_Write_Lock ()
	{
		bool te = false;
		if (!m_Write.compare_exchange_weak (te, true))
		{
			return ErrorCode::FAIL;
		}
		if (m_ReadNumber.load ())
		{
			m_Write.store (false);
			return ErrorCode::FAIL;
		}
		return ErrorCode::SUCCESS;
	}

	void RW_Lock::Read_Unlock ()
	{
		--m_ReadNumber;
	}

	void RW_Lock::Write_Unlock ()
	{
		m_Write.store (false);
	}

	//R_Lock
	R_Lock::R_Lock (RW_Lock& v)
		:m_d (v)
	{
	}

	R_Lock::~R_Lock ()
	{
	}

	void R_Lock::lock ()
	{
		m_d.Read_Lock ();
	}

	void R_Lock::unlock ()
	{
		m_d.Read_Unlock ();
	}

	ErrorCode R_Lock::try_lock ()
	{
		return m_d.Try_Read_Lock ();
	}

	//W_Lock
	W_Lock::W_Lock (RW_Lock&v)
		:m_d (v)
	{

	}

	W_Lock::~W_Lock ()
	{

	}

	void W_Lock::lock ()
	{
		m_d.Write_Lock ();
	}

	void W_Lock::unlock ()
	{
		m_d.Write_Unlock ();
	}

	ErrorCode W_Lock::try_lock ()
	{
		return m_d.Try_Write_Lock ();
	}

	//S_Lock

	S_Lock::S_Lock ()
		:m_V (false)
	{

	}

	S_Lock::S_Lock (S_Lock&&aim)noexcept
	{
		m_V.exchange (aim.m_V.exchange (m_V));
	}

	S_Lock& S_Lock::operator= (S_Lock&&aim)noexcept
	{
		m_V.exchange (aim.m_V.exchange (m_V));
		return *this;
	}

	void S_Lock::lock ()
	{
		bool te = false;
		while (!m_V.compare_exchange_weak (te, true))
		{
			te = false;
			_WAIT_TIME (1, ms);
		}
	}

	bool S_Lock::try_lock ()
	{
		bool te = false, result;
		result = m_V.compare_exchange_weak (te, true);
		return result;
	}

	void S_Lock::unlock ()
	{
		m_V.store (false);
	}

	//IdentityNumber

	std::atomic<size_t> IdentityNumber::SM_NumberMaker;

	IdentityNumber::IdentityNumber ()
		:m_number (SM_NumberMaker++)
	{

	}

	IdentityNumber::~IdentityNumber ()
	{
		//不用归还，64位的数据不考虑能被用完。
	}

	IdentityNumber::IdentityNumber (IdentityNumber&&aim)noexcept
	{
		std::swap (aim.m_number, m_number);
	}

	size_t IdentityNumber::GetNumber ()
	{
		return m_number;
	}

	void IdentityNumber::operator=(IdentityNumber&&aim)noexcept
	{
		std::swap (aim.m_number, m_number);
	}

	bool IdentityNumber::operator==(const IdentityNumber&aim)
	{
		return m_number == aim.m_number;
	}

	bool IdentityNumber::operator<(const IdentityNumber&aim)
	{
		return m_number < aim.m_number;
	}

	bool IdentityNumber::operator>(const IdentityNumber&aim)
	{
		return m_number > aim.m_number;
	}

	//Lock_Memger

	thread_local Lock_Memger Lock_Memger::TM_LM;

	Lock_Memger::~Lock_Memger ()
	{
		for (auto&var : m_lock_pool)
		{
			var->unlock ();
		}
	}

	void Lock_Memger::Push (lock_base*aim)
	{
		size_t index = m_lock_pool.size ();//记录解锁解到那了。
		auto NowTre = m_lock_pool.begin ();
		for (int i = m_lock_pool.size () - 1; i >= 0; i--)
		{
			auto now = (m_lock_pool[i]);
			if (*now > (*aim))
			{
				#ifdef SWD_LOCK_TEST  //是否检测有无因调用导致的死锁
				assert (false);//你的线程有死锁。
				#endif // _DEBUG

				--index;
				now->unlock ();//解锁比自己大的锁
			}
			else if (*now == (*aim))
			{
				#ifdef SWD_LOCK_TEST  //是否检测有无因调用导致的死锁
				assert (false);//你的线程有死锁。
				#endif // _DEBUG

				break;
			}
			else
			{
				NowTre += i;
				m_lock_pool.insert (NowTre, aim);//插入队列
				break;
			}
		}
		for (size_t i = index; i < m_lock_pool.size (); i++)
		{
			auto now = (m_lock_pool[i]);
			now->lock ();//重新加锁
		}
	}

	void Lock_Memger::Pop (lock_base*aim)
	{
		auto NowTre = m_lock_pool.begin ();
		size_t size = m_lock_pool.size ();
		for (; NowTre != m_lock_pool.end (); ++NowTre)//解锁不用理会顺序
		{
			if ((**NowTre) == (*aim))
			{
				aim->unlock ();
				m_lock_pool.erase (NowTre);//修改容器后直接返回，所以不会影响迭代器正确性
				return;
			}
		}
		if ((NowTre == m_lock_pool.end ()) && (size == m_lock_pool.size ()))
		{
			assert (false);//解锁一个未加锁的锁。
		}
	}

	void Lock_Memger::Pause ()
	{
		for (auto & var: m_lock_pool)
		{
			var->unlock ();
		}
	}

	void Lock_Memger::Regain ()
	{
		for (auto & var : m_lock_pool)
		{
			var->lock ();
		}
	}

	//Thread

	Thread::~Thread ()
	{

	}

	void Thread::join ()
	{
		if (std::thread::joinable ())
		{
			#ifdef SWD_LOCK_TEST  //是否检测有无因调用导致的死锁
			_WAIT_TIME (20, s);//因为是检测，所以无所谓时间有多久，就是看看子线程能不能结束，20秒都不结束，建议看看是不是死锁了。
			assert (!std::thread::joinable ());
			#endif // _DEBUG
			Lock_Memger::TM_LM.Pause ();
			std::thread::join ();
			Lock_Memger::TM_LM.Regain ();
		}
		else
		{
			std::thread::join ();
		}
	}
}
