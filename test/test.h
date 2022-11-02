#pragma once
#include<lock.h>

#include <ctime>
std::mutex pp;
std::vector<swd::mutex> lock_array;

std::atomic<int> thread_number = 100;

int LockNumber = 100;//锁数量
double CreateThreadRate = 0.7;//创子线程率
double Recursion = 0.8;//递归率
double ThreadWaitRate = 0.5;//线程合并率
double WorkOver = 0.1;//业务结束率
double LockRate = 0.9;//加锁率

std::atomic<int> MaxStackDeep = 0;//最大栈深
thread_local std::atomic<int> NowStackDeep = 0; //当前并行数

std::atomic<int> MaxParallel = 0;//最大并行数
thread_local std::atomic<int> NowParallel = 0; //当前并行数


class MakeRand
{
public:
	~MakeRand();
	static int Rand();
private:
	MakeRand();
	void iteration();
	static MakeRand m_self;
	int m_one;
	int m_two;
	int m_three;

};

MakeRand MakeRand::m_self;

MakeRand::MakeRand()
{
	std::srand(std::time(nullptr));
	m_one = rand();
	m_two = rand();
	m_three = rand();
}

inline void MakeRand::iteration()
{
	m_two = (m_one & m_three) << 2;
	m_three = m_one >> 3 + m_two << 2;
	std::srand(std::sqrt(m_three + m_two + m_two));
}

MakeRand::~MakeRand()
{
}

inline int MakeRand::Rand()
{
	m_self.iteration();
	return rand();
}

int64_t TrueNumber = 0;
int64_t FalseNumber = 0;

bool probabilities(const double& limit, const int RandValue = MakeRand::Rand())
{
	auto result = (RandValue % 100) < (int)(limit * 100);
	result ? ++TrueNumber : ++FalseNumber;
	return result;
}

void sun();

void Run()
{
	if ((thread_number--) > 0 && probabilities(CreateThreadRate))
	{ //随机创线程
		swd::thread t(sun);
		if (probabilities(ThreadWaitRate))
		{
			t.join();
		}
		else
		{
			t.detach();
		}
	}

	while (probabilities(WorkOver))
	{ //随机执行业务
		_WAIT_;
	}

	if (probabilities(Recursion))
	{
		if (NowStackDeep>30)
		{
			return;//避免栈溢出
		}
		++NowStackDeep;
		//更新最大栈深
		if (NowStackDeep > MaxStackDeep)
		{
			MaxStackDeep.store(NowStackDeep);
		}
		sun(); //随机递归
		--NowStackDeep;
	}
}

void sun()
{
	++NowParallel;
	try
	{
		if (probabilities(LockRate))
		{//是否加锁
			swd::lock_guard te_lo(lock_array[MakeRand::Rand() % LockNumber], lock_array[MakeRand::Rand() % LockNumber], lock_array[MakeRand::Rand() % LockNumber]);
			Run();
		}
		else
		{
			Run();
		}
	}
	catch (const std::exception& err)
	{
		std::cout << err.what() << std::endl;
	}
	//更新最大并行数
	if (NowParallel > MaxParallel)
	{
		MaxParallel.store(NowParallel);
	}

	NowParallel--;
}

void lock_test()
{
	try
	{
		lock_array.resize(LockNumber);
		int counter = 10000;
		while (counter--)
		{
			std::vector<swd::thread> t_te_l;
			for (size_t i = 0; i < 4; i++)
			{
				t_te_l.emplace_back(sun);
			}
			for (auto& var : t_te_l)
			{
				var.join();
			}
			t_te_l.clear();
			thread_number = 30;
			std::cout << "第" << 10000 - counter << "次完成" << std::endl;
		}
	}
	catch (const std::exception& err)
	{
		std::cout << err.what() << std::endl;
	}
	std::cout << "最大栈深：" << MaxStackDeep << "；最大并行数：" << MaxParallel << std::endl;
	std::cout << "随机真次数：" << TrueNumber << "；随机假次数：" << FalseNumber << std::endl;
}
