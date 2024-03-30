#include"CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::centralcache;
size_t CentralCache::centralcache_allocmemory_to_thread(void*& start, void*& end, size_t n, size_t size)
{
	size_t obj_index = alignfindbucket::find_bucket(size);
	assert(obj_index > 0);

	_spanlist[obj_index].locker.lock();

	span* span1 = getmemory_from_pagecache_or_itself_to_centralcache(_spanlist[obj_index], size);
	assert(span1);
	assert(span1->_freelist);

	size_t actual_return_num = 1;
	size_t i = 1;

	start = span1->_freelist;
	end = start;

	while (i < n && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		++actual_return_num;
		++i;
	}
	span1->_freelist = NextObj(end);
	NextObj(end) = nullptr;
	span1->usednum += actual_return_num;

	return actual_return_num;
}

span* CentralCache::getmemory_from_pagecache_or_itself_to_centralcache(SpanList& spanlist, size_t size)
{
	span* it = spanlist.Begin();
	while (it != spanlist.End())
	{
		if (!it->_freelist)
		{
			return it;
		}
		it = it->next;
	}

	spanlist.locker.unlock();

	PageCache::get_pagecache_instance()->locker.lock();

	span* newspan = PageCache::get_pagecache_instance()->allocmemory_from_pagecache_to_centralcache(memory_num_need_alloc(size));
	
	newspan->is_used =true;
	newspan->span_bytes = size;

	PageCache::get_pagecache_instance()->locker.unlock();

	char* start=(char*)(newspan->_pageId << PAGE_SIZE);
	size_t num=newspan->_pagenum << PAGE_SIZE;
	char* end = start + num;

	//切割从pagecache划分来的span（一整块）对象成每一块所需大小（size）挂载到freelist上
	newspan->_freelist = start;
	start += num;
	void* tail = newspan->_freelist;

	size_t i = 1;
	while (start < end)
	{
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}
	NextObj(tail) = nullptr;
	spanlist.locker.lock();
	spanlist.PushFront(newspan);
	return newspan;
}

void CentralCache::centralcache_releasrmemory(void* start, size_t size)
{
	assert(start);
	size_t obj_index = alignfindbucket::find_bucket(size);
	assert(obj_index);
	_spanlist->locker.lock();
	while (start)
	{
		void* next = NextObj(start);
		span* span1 = PageCache::get_pagecache_instance()->MapObjectToSpan(start);
		NextObj(start) = span1->_freelist;
		span1->_freelist = start;
		span1->usednum--;

		if (span1->usednum == 0)
		{
			_spanlist[obj_index].clearspan(span1);
			span1->_freelist = nullptr;
			span1->next = nullptr;
			span1->prev = nullptr;

			_spanlist[obj_index].locker.unlock();

			//回收到page cache
			PageCache::get_pagecache_instance()->locker.lock();
			PageCache::get_pagecache_instance()->ReleaseSpanToPageCache(span1);
			PageCache::get_pagecache_instance()->locker.unlock();

			_spanlist[obj_index].locker.lock();
		}
		start = next;
	}
	_spanlist[obj_index].locker.unlock();
	
}