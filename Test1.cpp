//#include "ObjectPool.h"
#include <iostream>
#include <vector>
#include <thread>
#include <stdlib.h>
#include "concurrentAlloc.h"
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

//struct TreeNode
//{
//	int _val;
//	TreeNode* _left;
//	TreeNode* _right;
//	TreeNode()
//		:_val(0)
//		, _left(nullptr)
//		, _right(nullptr)
//	{}
//	
//};
//
//void TestObjectPool()
//{
//	// 申请释放的轮次
//	const size_t Rounds = 3;
//	// 每轮申请释放多少次
//	const size_t N = 100000;
//	
//	std::vector<TreeNode*> v1;
//	v1.reserve(N);
//	size_t begin1 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v1.push_back(new TreeNode);
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			delete v1[i];
//		}
//		v1.clear();
//	}
//	size_t end1 = clock();
//	ObjectPool<TreeNode> TNPool;
//	
//	std::vector<TreeNode*> v2;
//	v2.reserve(N);
//	size_t begin2 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v2.push_back(TNPool.New());
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			TNPool.Delete(v2[i]);
//		}
//		v2.clear();
//	}
//	size_t end2 = clock();
//	cout << "new cost time:" << end1 - begin1 << endl;
//	cout << "object pool cost time:" << end2 - begin2 << endl;
//}
//void test()
//{
//	void* ptr1 = concurrentalloc(6);
//	void* ptr2 = concurrentalloc(8);
//
//}
//int main()
//{
//	//TestObjectPool();
//	//thread t1([]() {
//	//	for (int i = 0; i < 5; i++)
//	//	{
//	//		void* ptr=concurrentalloc(5);
//	//	}
//
//	//	});
//	//thread t2([]() {
//	//	for (int i = 0; i < 5; i++)
//	//	{
//	//		void* ptr = concurrentalloc(5);
//	//	}
//	//	});
//	//t1.join();
//	//t2.join();
//	test();
//	return 0;
//}

void test()
{
	//void* ptr1=  concurrentalloc(8);
	//void* ptr2 = concurrentalloc(8);
	//void* ptr3 = concurrentalloc(8);
	//void* ptr4 = concurrentalloc(8);
	//void* ptr5 = concurrentalloc(8);

	//concurrentfree(ptr1, 8);
	//concurrentfree(ptr2, 8);
	//concurrentfree(ptr3, 8);
	//concurrentfree(ptr4, 8);
	//concurrentfree(ptr5, 8);
	vector<void*> v;
	for (int i = 1; i < 50000; i++)
		v.push_back(concurrentalloc(i));
	//Sleep(10000);
	for (auto x : v)
	{
		concurrentfree(x);
	}
	cout << "hh" << endl;
}
void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&, k]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(malloc(16));
					v.push_back(malloc((16 + i) % 8192 + 1));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				size_t end2 = clock();
				v.clear();
				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮次malloc %u次: 花费：%u ms\n", nworks, rounds, ntimes, (size_t)malloc_costtime);
	printf("%u个线程并发执行%u轮次，每轮次free %u次: 花费：%u ms\n", nworks, rounds, ntimes, (size_t)free_costtime);
	printf("%u个线程并发malloc&free %u次，总计花费：%u ms\n", nworks, nworks * rounds * ntimes, (size_t)malloc_costtime + free_costtime);
}
// 单轮次申请释放次数 线程数 轮次
void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(concurrentalloc(16));
					v.push_back(concurrentalloc((16 + i) % 8192 + 1));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					concurrentfree(v[i]);
				}
				size_t end2 = clock();
				v.clear();
				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮次concurrent alloc %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, (size_t)malloc_costtime);
	printf("%u个线程并发执行%u轮次，每轮次concurrent dealloc %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, (size_t)free_costtime);
	printf("%u个线程并发concurrent alloc&dealloc %u次，总计花费：%u ms\n",
		nworks, nworks * rounds * ntimes, (size_t)malloc_costtime + free_costtime);
}
int main()
{
	size_t n = 10000;
	cout << "==========================================================" <<
		endl;
	BenchmarkConcurrentMalloc(n, 4, 10);
	cout << endl << endl;
	BenchmarkMalloc(n, 4, 10);
	cout << "==========================================================" <<
		endl;
	//test();
	return 0;
}
//int main()
//{
//	//test();
//	return 0;
//}