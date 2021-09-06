#pragma once
#include"Head.h"
#include"ErrorDef.h"

/*
读写锁，由W_Lock和R_Lock支持栈管理。
无死锁组件：接口：Lock_Guard 和 S_Lock。由编号和线程容器管理。

定义宏SWD_LOCK_TEST可以开启死锁检测，死锁不一定会触发，但可以检测。运行时检测。当然，如果性能敏感，可以开启检测做测试，修改死锁代码来获得性能，如无必要可以关闭检测。
	原理：
		死锁有两种：线程间，线程内。
			线程间死锁：1持有A获取B，2持有B获取A。
				如果AB有序，规定必须从小向大获取。那假设A<B，那么2就必须先解锁B才能获取A之后再获取B。此时触发解锁操作，那就是死锁，上面的宏定义会在此时报错。
			线程内死锁：1持有A，1调用2,2获取A。
				因为1和2在同一个线程，所以1持有A就是2持有A，所以此时的加锁请求会忽略。
			特殊情况：如果1中开线程执行2，并且1已经持有A，且2获取A，1在之后等待2的执行结束。此时还会死锁。但这样的代码我不知道开线程干什么？可能为了节省中间那段指令的时间，但又要求某处必须结束。这种情况的死锁，需要系统和语言层面共同支持。
					那：所有线程都分离，就不会有上面的情况。
						可以包装std::thread，在调用join向子线程授权所持有的全部锁（或者释放所有锁，但执行完之后又全部加回来）。
			如果创建一个线程没有join或者detach那这个创建的线程本身就不安全。这种情况不考虑
*/

namespace swd
{
	//拷贝，移动时不支持并发访问。
	class R_Lock;
	class W_Lock;
	class _EXPORTING RW_Lock
	{
	public:
		RW_Lock ();
		~RW_Lock ();//会阻塞，直到这个锁的所有访问全部结束。
	protected:
		friend R_Lock;
		friend W_Lock;
		void Read_Lock ();//会阻塞
		void Write_Lock ();//会阻塞
		ErrorCode Try_Read_Lock ();//不阻塞
		ErrorCode Try_Write_Lock ();//不阻塞
		void Read_Unlock ();
		void Write_Unlock ();
	private:
		/*
			读写锁需要两个原子来做，解决读连续导致的写阻塞，两个可以有一定的随机性。
			平均看不会引起阻塞，写的优先级更高，但也不能形成写连续。
			保证写时读都结束，读时写都不在执行。
		*/
		std::atomic<uint32_t> m_ReadNumber;//读取
		std::atomic<bool> m_Write;//写入
	};

	//包一次，为了能用栈变量保护
	class _EXPORTING R_Lock
	{
	public:
		R_Lock (RW_Lock&);
		~R_Lock ();
		void lock ();
		void unlock ();
		ErrorCode try_lock ();
	private:
		RW_Lock &m_d;
	};

	class _EXPORTING W_Lock
	{
	public:
		W_Lock (RW_Lock&);
		~W_Lock ();
		void lock ();
		void unlock ();
		ErrorCode try_lock ();
	private:
		RW_Lock &m_d;
	};

	//序号
	class IdentityNumber
	{
	public:
		IdentityNumber ();
		IdentityNumber (IdentityNumber&&)noexcept;
		virtual ~IdentityNumber ();
		size_t GetNumber ();
		void operator=(IdentityNumber&&)noexcept;
		virtual bool operator==(const IdentityNumber&);
		virtual bool operator<(const IdentityNumber&);
		virtual bool operator>(const IdentityNumber&);
	private:
		IdentityNumber& operator=(const IdentityNumber&) = delete;
		IdentityNumber (const IdentityNumber&) = delete;
		size_t m_number;//因为唯一修改就是构造时，所以不用原子
		static std::atomic<size_t> SM_NumberMaker;
	};

	/*
		锁可能因为调用导致死锁，即由多个Lock_Guard在同一个线程上导致死锁。
		此时应该由一个thread_local来管理线程上的锁。那么所有的加锁操作都是向线程上的管理者申请。这样就不会再同一个线程上出现死锁。
		那么期望的调用就是 std::Lock_Guard<std::mutex> lock(te);这样锁的申请就在栈上。
		综上：可以解决死锁。
	*/
	//可以通过基类来实现各种锁。

	class lock_base :public IdentityNumber
	{
	public:
		lock_base () = default;
		lock_base (lock_base&&) = default;
		virtual ~lock_base () = default;
		virtual void lock () = 0;
		virtual void unlock () = 0;
		virtual bool try_lock () = 0;
	private:

	};

	/*c++自己的不支持跨线程操作。
	一个线程加锁一个线程解锁会报错，这个不会。
	并且支持c++的锁管理对象，例如：std::lock_guard<S_Lock>lock (m_FileHead_lock);
注意：移动语义需要有其他锁来保护才能安全使用，类本身不提供保护。
	c++20标准，可以添加等待和唤起操作的函数。目前仅是对标c++11标准。
*/
//互斥锁
	class _EXPORTING S_Lock :public lock_base
	{
	public:
		S_Lock ();
		~S_Lock () = default;
		S_Lock (S_Lock&&)noexcept;
		S_Lock& operator= (S_Lock&&)noexcept;
		void lock ();
		bool try_lock ();
		void unlock ();
	private:
		S_Lock (S_Lock&) = delete;
		S_Lock& operator= (S_Lock&) = delete;
		std::atomic<bool> m_V;
	};


	class Lock_Memger
	{//不极端情况，开销不大。十几个锁排序拷贝重加锁等等，不会太费事。
	public:
		Lock_Memger () = default;
		~Lock_Memger ();
		void Push (lock_base*);
		void Pop (lock_base*);
		//暂停线程上所有的锁。期望暂停恢复成对出现，或者别用。
		void Pause ();
		//恢复线程上所有的锁。
		void Regain ();
		static thread_local Lock_Memger TM_LM;
	private:
		Lock_Memger (Lock_Memger&&) = delete;
		Lock_Memger (const Lock_Memger&) = delete;
		std::vector<lock_base*> m_lock_pool;
	};

	//锁必须继承lock_base
	class _EXPORTING Lock_Guard
	{
	public:
		template<typename ...T>
		Lock_Guard (lock_base &now, T&& ...args)
		{
			initialise (std::forward<T> (args)...);
			for (auto&var : m_lock_pool)
			{
				Lock_Memger::TM_LM.Push (var);
			}
		}

		~Lock_Guard ()
		{
			for (auto&var : m_lock_pool)
			{
				Lock_Memger::TM_LM.Pop (var);
			}
		}

	private:
		template<typename ...T>
		void initialise (lock_base &now, T&& ...args)
		{
			m_lock_pool.push_back (&now);
			initialise (std::forward<T> (args)...);
		}
		void initialise (lock_base &now)
		{
			m_lock_pool.push_back (&now);
		}
		void initialise ()
		{

		}
		std::vector<lock_base*> m_lock_pool;
	};

	//改写标准库的线程，修改join的实现。
	class Thread :public std::thread
	{//基类子类一样大，所以不会内存泄漏
	public:
		Thread (const Thread&) = delete;
		template<typename ...T>
		Thread (T&& ... args)
			:std::thread (std::forward<T> (args)...)
		{

		}
		Thread (Thread&& _Other) noexcept
			:std::thread (std::forward<std::thread> (_Other))
		{

		}
		Thread (Thread* _Other) = delete;
		~Thread ();
		void join ();
		Thread& operator=(Thread&& _Other) noexcept
		{	// move from _Other
			std::thread::operator=(std::forward<std::thread> (_Other));
			return *this;
		}
		Thread& operator=(const Thread& _Other) = delete;
		Thread& operator=(Thread* _Other) = delete;
	};

}
