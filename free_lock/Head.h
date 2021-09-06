#pragma once
#include<string>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include<stdexcept>
#include<cstdlib>
#include<cmath>
#include<fstream>
#include<cassert>
#include<atomic>
#include<thread>
#include<chrono>
#include<iomanip>
#include<ctime>
#include<deque>
#include<mutex>
#include<iostream>
#include<sstream>
#include<initializer_list>
#include<algorithm>

# if defined(DLL_EXPORTING)
#  define _EXPORTING __declspec(dllexport)
# else					 
#  define _EXPORTING __declspec(dllimport)
# endif

//用法：_WAIT_TIME(1,ms); 暂停1ms。相应的，还可以： 	_WAIT_TIME(20,s);暂停1s。
#define _WAIT_TIME(length,unit) \
using namespace std::chrono;\
std::this_thread::sleep_for (length##unit);

#define _WAIT_ \
 _WAIT_TIME(30,ms);
