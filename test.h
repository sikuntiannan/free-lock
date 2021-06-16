#pragma once
#include<Lock.h>
#include<Log.h>
#include <ctime>
using namespace swd;

std::vector<S_Lock> lock_array;

std::atomic<int> thread_number = 100;

void sun ()
{
	std::srand (std::time (nullptr));
	int v = std::rand ();
	if (v & 0x00001)
	{
		int n = v % 100;
		Lock_Guard te_l (lock_array[n]);

		v = std::rand ();
		if ((thread_number--)>0 && (v & 0x00001))
		{
			Thread t (sun);
			v = std::rand ();
			if (v & 0x00001)
			{
				t.join ();
			}
			else
			{
				t.detach ();
			}
		}
	}
	else
	{
		v = std::rand ();
		if ((thread_number--)>0 && (v & 0x00001))
		{
			Thread t (sun);
			v = std::rand ();
			if (v & 0x00001)
			{
				t.join ();
			}
			else
			{
				t.detach ();
			}
		}
	}
	v = std::rand ();
	while((v & 0x00001))
	{
		v = std::rand ();
		_WAIT_;
	}
}

void lock_test()
{
	try
	{
		lock_array.resize (100);
		int counter = 10000;
		while (counter--)
		{
			std::vector<Thread> t_te_l;
			for (size_t i = 0; i < 4; i++)
			{
				t_te_l.emplace_back (sun);
			}
			for (auto &var : t_te_l)
			{
				var.join ();
			}
			t_te_l.clear ();
			thread_number = 100;
			std::cout << "第" << 10000 - counter << "次完成" << std::endl;
		}
	}
	catch (const std::exception&err)
	{
		std::cout << err.what () << std::endl;
	}
	
}
