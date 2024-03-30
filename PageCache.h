#pragma once
#include"Common.h"
#include"ObjectPool.h"
#include"PageMap.h"
class PageCache 
{
private:
	static PageCache pagecache;
	Objectpool<span> memory_pool;
	TCMalloc_PageMap1<32-PAGE_SIZE> pagehash;
	SpanList pagelist[PAGE_NUM];
private:
	PageCache() {}
	PageCache(const PageCache& obj) = delete;
	PageCache& operator=(PageCache& obj) = delete;
public:
	std::mutex locker;
	
	static PageCache* get_pagecache_instance()
	{
		return &pagecache;
	}

	span* MapObjectToSpan(void* obj);

	span* allocmemory_from_pagecache_to_centralcache(size_t size);

	void ReleaseSpanToPageCache(span* span1);
};