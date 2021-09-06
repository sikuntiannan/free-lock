#pragma once
#include"Head.h"

namespace swd
{
	//链队列，线程安全，插入和删除可以并行，无锁结构。
	//T需要可以移动
	template<typename T>
	class Queue
	{
		struct Node
		{
			Node (T&&aim);
			T m_V;
			std::atomic<Node*> m_Next = nullptr;
		};
	public:
		Queue ();
		~Queue ();
		void push (T&& );
		std::unique_ptr<T> pop ();
		size_t size ();
		void clear ();
	private:
		std::mutex m_PushLock;
		std::mutex m_PopLock;
		std::atomic<Node*> m_begin;
		std::atomic<Node*> m_end;
		std::atomic<size_t> m_size;
	};

	template<typename T>
	Queue<T>::Node::Node (T&&aim)
		:m_V (std::forward<T>(aim))
	{
		
	}

	template<typename T>
	Queue<T>::Queue ()
	{
		m_size.store (0);
		m_begin.store (nullptr);
		m_end.store (nullptr);
	}

	template<typename T>
	Queue<T>::~Queue ()
	{
		m_PushLock.lock ();
		m_PopLock.lock ();
		std::atomic<Node*> now;
		now.store (m_begin);
		while (now.load () != nullptr)
		{
			std::atomic<Node*>te;
			te.store (now.load ()->m_Next);
			delete now.load ();
			now.store (te);
		}
	}

	template<typename T>
	void Queue<T>::push (T&& aim)
	{
		Node* ptr = new Node (std::forward<T>(aim)), *null = nullptr;
		{
			std::lock_guard<std::mutex> te (m_PushLock);
			if (m_end.load () == nullptr)
			{

			}
			m_end.load ()->m_Next = ptr;
			m_end.store (ptr);
		}
		m_begin.compare_exchange_weak (null, ptr);
		++m_size;
	}

	template<typename T>
	std::unique_ptr<T> Queue<T>::pop ()
	{
		std::unique_ptr<T> result=nullptr;
		Node* ptr = nullptr;
		{
			std::lock_guard<std::mutex> te (m_PopLock);
			if (m_size.load () > 0)
			{
				ptr = m_begin.load ();
				m_begin.store (m_begin.load ()->m_Next);
				--m_size;
			}
		}
		if (ptr != nullptr)
		{
			result = std::make_unique<T> (ptr->m_V);
		}
		return result;
	}

	template<typename T>
	size_t Queue<T>::size ()
	{
		return m_size.load ();
	}

	template<typename T>
	void Queue<T>::clear ()
	{
		std::lock_guard<std::mutex> te (m_PushLock);
		std::lock_guard<std::mutex> te (m_PopLock);
		std::atomic<Node*> now = m_begin;
		while (now.load () != nullptr)
		{
			std::atomic<Node*>te = now.load ()->m_Next;
			delete now.load ();
			now = te;
		}
		m_size.store (0);
		m_begin.store (nullptr);
		m_end.store (nullptr);
	}
}
