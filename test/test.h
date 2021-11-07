#pragma once
#include<Lock.h>
#include <ctime>
using namespace swd;

std::vector<S_Lock> lock_array;

std::atomic<int> thread_number = 30;

thread_local int Stacknumber = 0;

void sun ()
{
	Stacknumber++;
	try
	{
		std::srand (std::time (nullptr));
		int v = std::rand ();
		if (v & 0x00001)
		{
			int n = v % 100;
			lock_guard te_lo (lock_array[n]);
			v = std::rand ();
			n = v % 100;
			lock_guard te_lt (lock_array[n]);
			v = std::rand ();
			n = v % 100;
			lock_guard te_lth (lock_array[n]);
			if ((thread_number--) > 0 && (v & 0x00001))
			{
				thread t (sun);
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
			v = std::rand ();
			while ((v & 0x00001))
			{
				v = std::rand ();
				_WAIT_;
			}
			v = std::rand ();
			if (Stacknumber <10 &&(v & 0x00001))
			{
				sun ();
			}
		}
		else
		{
			v = std::rand ();
			if ((thread_number--) > 0 && (v & 0x00001))
			{
				v = std::rand ();
				while ((v & 0x00001))
				{
					v = std::rand ();
					_WAIT_;
				}

				thread t (sun);
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
	}
	catch (const std::exception&err)
	{
		std::cout << err.what () << std::endl;
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
			std::vector<thread> t_te_l;
			for (size_t i = 0; i < 4; i++)
			{
				t_te_l.emplace_back (sun);
			}
			for (auto &var : t_te_l)
			{
				var.join ();
			}
			t_te_l.clear ();
			thread_number = 30;
			std::cout << "第" << 10000 - counter << "次完成" << std::endl;
		}
	}
	catch (const std::exception&err)
	{
		std::cout << err.what () << std::endl;
	}
	
}
