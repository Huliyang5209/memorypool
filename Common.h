#pragma once
#include <memory>
#include <cassert>
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <map>

#define PAGE_SIZE 13
#define MAX_BYTE 256*1024
#define BUCKET_NUM 208
#define PAGE_NUM 129


#ifdef _WIN32
#include <Windows.h>
#else
// linux下brk mmap等
#endif


#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#else
//linux
#endif

inline static void* systemalloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
#else
	// linux下brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}

inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap等
#endif
}

inline static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

class FreeList
{
private:
	size_t nodenums=0;
	void* _freelist=nullptr;
	size_t requsetnum;
	size_t _neednum = 1;
public:
	void push(void* obj)
	{ 
		assert(obj);
		NextObj(obj) = _freelist;
		_freelist = obj;
		nodenums++;
	}
	void* pop()
	{
		void* ptr;
		ptr = _freelist;
		_freelist = NextObj(ptr);
		nodenums--;
		return ptr;
	}
	bool empty()
	{
		return _freelist == nullptr;
	}
	size_t NodeNum()
	{
		return nodenums;
	}
	size_t& getneednum()
	{
		return _neednum;
	}
	void pushmemory_from_CentralCache_to_ThreadCache(void* start, void* end,size_t n)
	{
		
		NextObj(end) = _freelist;
		_freelist = start;
		nodenums+=n;
	}
	void popmemory_from_ThreadCache_to_CentralCache(void*& start, void*& end, size_t n)
	{
		start = _freelist;
		end = start;

		for (int i = 0; i < n; i++)
		{
			end = NextObj(end);
		}

		_freelist = end;
		NextObj(end) = nullptr;
		nodenums -= n;
	}
};

class alignfindbucket 
{
public:
	static inline size_t _alignSize(size_t size,size_t align)
	{
		return (((size)+align-1) & ~(align-1));
	}
	static inline size_t _findBucket(size_t size,size_t align_shift)
	{
		return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
	}
	static inline size_t memory_alignment(size_t size)
	{
		if (size <= 128)
		{
			return _alignSize(size, 8);
		}
		else if (size <= 1024)
		{
			return _alignSize(size, 16);

		}
		else if (size <= 8 * 1024)
		{
			return _alignSize(size, 128);

		}
		else if (size <= 64 * 1024)
		{
			return _alignSize(size, 1024);

		}
		else if (size <= 256 * 1024)
		{
			return _alignSize(size, 8 * 1024);

		}
		else
		{
			//大块内存
			return _alignSize(size, 1 << PAGE_SIZE);
		}
	}
	static inline size_t find_bucket(size_t size)
	{
		assert(size <= MAX_BYTE);
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (size <= 128) {
			return _findBucket(size, 3);
		}
		else if (size <= 1024) {
			return _findBucket(size - 128, 4) + group_array[0];
		}
		else if (size <= 8 * 1024) {
			return _findBucket(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 64 * 1024) {
			return _findBucket(size - 8 * 1024, 10) + group_array[2] + group_array[1]
				+ group_array[0];
		}
		else if (size <= 256 * 1024) {
			return _findBucket(size - 64 * 1024, 13) + group_array[3] +
				group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}
		return -1;
	}	
};

 static size_t obj_num_need_central_alloc(size_t size)
{
	 assert(size > 0);

	 size_t num = MAX_BYTE / size;
	 if (num < 2)
	 {
		 num=2;
	 }
	 if (num > 512)
	 {
		 num = 512;
	 }

	return num;
}

 struct span
 {
	 void* _freelist = nullptr;

	 struct span* next;
	 struct span* prev;

	 PAGE_ID _pageId = 0;
	 size_t _pagenum = 0;

	 size_t usednum = 0;
	 size_t span_bytes=0;

	 bool is_used = false;
 };
 class SpanList 
 {
 public:
	 std::mutex locker;
 private:
	 span* _head;
 public:
	 SpanList()
	 {
		 _head = new span();
		 _head->next = _head;
		 _head->prev = _head;
	 }
	 void addspan(span* currentnode,span* newnode) 
	 {
		 assert(currentnode);
		 assert(newnode);
		 span* pos = currentnode->prev;

		 pos->next = newnode;
		 newnode->prev = pos;

		 newnode->next = currentnode;
		 currentnode->prev = newnode;

	 }
	 void PushFront(span* newspan)
	 {
		 addspan(Begin(), newspan);
	 }
	 void clearspan(span* currentnode)
	 {
		 assert(currentnode);
		 assert(currentnode != _head);
		 span* pos_next = currentnode->next;
		 span* pos_prev = currentnode->prev;

		 pos_next->prev = pos_prev;
		 pos_prev->next = pos_next;
		 
	 }
	 span* PopFront()
	 {
		 span* ret = Begin();
		 clearspan(ret);
		 return ret;
	 }
	 span* Begin()
	 {
		 return _head->next;
	 }
	 span* End()
	 {
		 return _head->prev;
	 }
	 bool Empty()
	 {
		 return _head->next == _head;
	 }
 };

  static size_t memory_num_need_alloc(size_t size)
{
	 size_t k = obj_num_need_central_alloc(size);
	 size_t need_page = k * size;
	 need_page>>= PAGE_SIZE;
	 if (need_page == 0)
	 {
		 need_page = 1;
	 }
	 return need_page;
}