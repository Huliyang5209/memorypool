#pragma once
#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"

static void* concurrentalloc(size_t size)
{
	if (size > MAX_BYTE)
	{
		//对齐数
		size_t actualSize = alignfindbucket::memory_alignment(size);
		//页的个数
		size_t PageNum = actualSize >> PAGE_SIZE;

		//访问page cache要进行加锁
		PageCache::get_pagecache_instance()->locker.lock();
		span* span1 = PageCache::get_pagecache_instance()->allocmemory_from_pagecache_to_centralcache(PageNum);
		span1->span_bytes = size;
		PageCache::get_pagecache_instance()->locker.unlock();
		//获得地址
		void* ptr = (void*)(span1->_pageId << PAGE_SIZE);

		return ptr;

	}
	else
	{
		//设置成静态，其他线程来申请的时候，也只存在一份。减少浪费
		static Objectpool<ThreadCache> objPool;
		if (ptls_threadcache == nullptr)
			ptls_threadcache = objPool.New();
		//std::cout <<"分配" << std::this_thread::get_id() << std::endl;
		return ptls_threadcache->alloc_thread_memory(size);
	}

}
static void concurrentfree(void* ptr)
{
	assert(ptr);
	//获取到对应的span
	span* span1 = PageCache::get_pagecache_instance()->MapObjectToSpan(ptr);
	size_t size = span1->span_bytes;

	if (size > MAX_BYTE)
	{
		//访问page cache要加锁
		PageCache::get_pagecache_instance()->locker.lock();
		PageCache::get_pagecache_instance()->ReleaseSpanToPageCache(span1);
		PageCache::get_pagecache_instance()->locker.unlock();
	}
	else
	{
		ptls_threadcache->dealloc_memory(ptr, size);
	}

}