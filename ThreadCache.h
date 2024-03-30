#pragma once
#include"Common.h"
class ThreadCache
{
private:
	FreeList _hashbucket[BUCKET_NUM];
public:
	void* alloc_thread_memory(size_t threadsize);
	void* To_central_requset_memory(size_t index,size_t requestmemory);

	void dealloc_memory(void* obj,size_t size);
	void return_memory_to_centralcache(FreeList& freelist,size_t size);
};

static __declspec(thread) ThreadCache* ptls_threadcache = nullptr;
