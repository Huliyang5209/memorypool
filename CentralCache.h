#pragma once
#include"Common.h"

class CentralCache
{
private:
static CentralCache centralcache;
SpanList _spanlist[BUCKET_NUM];
public:
	static CentralCache* get_centralcache_instance()
	{
		return &centralcache;
	}

	size_t centralcache_allocmemory_to_thread(void*& start, void*& end,size_t num,size_t size);

	span* getmemory_from_pagecache_or_itself_to_centralcache(SpanList& spanlist,size_t size);

	void  centralcache_releasrmemory(void* start, size_t size);
private:
	//单例模式，构造私有，拷贝、赋值禁掉
	CentralCache() {}
	CentralCache(const CentralCache& obj) = delete;
	CentralCache& operator=(CentralCache& obj) = delete;
};