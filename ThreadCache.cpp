#include"ThreadCache.h"
#include"CentralCache.h"
void* ThreadCache::alloc_thread_memory(size_t size)
{
	 size_t obj_memory_align = alignfindbucket::memory_alignment(size);
	 size_t obj_memory_index = alignfindbucket::find_bucket(size);

	 assert(obj_memory_index > 0);
	 
	 if (_hashbucket[obj_memory_index].empty())
	 {
		 return To_central_requset_memory(obj_memory_index, size);
	 }

	 return _hashbucket[obj_memory_index].pop();
}

void* ThreadCache::To_central_requset_memory(size_t index, size_t requestmemory)
{
	
	size_t request_central_num =min(_hashbucket[index].getneednum(),obj_num_need_central_alloc(requestmemory));
	if (request_central_num == _hashbucket[index].getneednum());
	{
		_hashbucket[index].getneednum() += 2;
	}

	void* start;
	void* end;
	size_t actual_return_num = CentralCache::get_centralcache_instance()->centralcache_allocmemory_to_thread(start,end,request_central_num,requestmemory);

	assert(actual_return_num >= 1);
	assert(start);
	assert(end);

	if (actual_return_num == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
      _hashbucket[index].pushmemory_from_CentralCache_to_ThreadCache(NextObj(start),end,actual_return_num-1);
	}
}

void ThreadCache::dealloc_memory(void* obj,size_t size)
{   
	assert(obj);
	assert(size <= MAX_BYTE);
	size_t obj_memory_index= alignfindbucket::find_bucket(size);
	_hashbucket[obj_memory_index].push(obj);
	
	if (_hashbucket[obj_memory_index].NodeNum() > _hashbucket[obj_memory_index].getneednum())
	{
		return_memory_to_centralcache(_hashbucket[obj_memory_index], size);
	}
}

void ThreadCache::return_memory_to_centralcache(FreeList& freelist,size_t size)
{
	void* start;
	void* end;
	
	freelist.popmemory_from_ThreadCache_to_CentralCache(start, end, freelist.getneednum());
	
	CentralCache::get_centralcache_instance()->centralcache_releasrmemory(start, size);
}