#pragma once
#include "Common.h"
#include "ThreadCache.h"
#include "PageCache.h"

static void* concurrentalloc(size_t size)
{
	if (size > MAX_BYTE)
	{
		//������
		size_t actualSize = alignfindbucket::memory_alignment(size);
		//ҳ�ĸ���
		size_t PageNum = actualSize >> PAGE_SIZE;

		//����page cacheҪ���м���
		PageCache::get_pagecache_instance()->locker.lock();
		span* span1 = PageCache::get_pagecache_instance()->allocmemory_from_pagecache_to_centralcache(PageNum);
		span1->span_bytes = size;
		PageCache::get_pagecache_instance()->locker.unlock();
		//��õ�ַ
		void* ptr = (void*)(span1->_pageId << PAGE_SIZE);

		return ptr;

	}
	else
	{
		//���óɾ�̬�������߳��������ʱ��Ҳֻ����һ�ݡ������˷�
		static Objectpool<ThreadCache> objPool;
		if (ptls_threadcache == nullptr)
			ptls_threadcache = objPool.New();
		//std::cout <<"����" << std::this_thread::get_id() << std::endl;
		return ptls_threadcache->alloc_thread_memory(size);
	}

}
static void concurrentfree(void* ptr)
{
	assert(ptr);
	//��ȡ����Ӧ��span
	span* span1 = PageCache::get_pagecache_instance()->MapObjectToSpan(ptr);
	size_t size = span1->span_bytes;

	if (size > MAX_BYTE)
	{
		//����page cacheҪ����
		PageCache::get_pagecache_instance()->locker.lock();
		PageCache::get_pagecache_instance()->ReleaseSpanToPageCache(span1);
		PageCache::get_pagecache_instance()->locker.unlock();
	}
	else
	{
		ptls_threadcache->dealloc_memory(ptr, size);
	}

}