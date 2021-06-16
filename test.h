#pragma once
#include<Lock.h>
#include<Log.h>
#include <ctime>
using namespace swd;

std::vector<S_Lock> lock_array;



void sun ()
{
	std::srand (std::time (nullptr));
	int v = std::rand ();
	if (v & 0x00001)
	{
		int n = v % 100;
		Lock_Guard te_l (lock_array[n]);

		int v = std::rand ();
		if (v & 0x00001)
		{
			Thread t (sun);

			int v = std::rand ();
			if (v & 0x00001)
			{
				t.join ();
			}
		}
	}
	else
	{
		int v = std::rand ();
		if (v & 0x00001)
		{
			Thread t (sun);

			int v = std::rand ();
			if (v & 0x00001)
			{
				t.join ();
			}
		}
	}

}

void lock_test()
{
	lock_array.resize (100);
	int counter = 10000;
	while (counter--)
	{
		std::vector<Thread> t_te_l;
		for (size_t i = 0; i < 50; i++)
		{
			t_te_l.emplace_back (sun);
		}
		for (auto &var : t_te_l)
		{
			var.join ();
		}
		std::cout << "第"<< 10000 -counter<<"次完成"<< std::endl;
	}
}
