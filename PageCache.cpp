#include"PageCache.h"
#include "Common.h"

PageCache PageCache::pagecache;
span* PageCache::allocmemory_from_pagecache_to_centralcache(size_t k)
{
    assert(k > 0);
    if (k > PAGE_NUM-1)
    {
        span* spans = memory_pool.New();
        void* heap_address = systemalloc(k);

        spans->_pageId= (PAGE_ID)heap_address >> PAGE_SIZE;
        spans->_pagenum = k;

        pagehash.set(spans->_pageId, spans);
        return spans;

    }
    if (!pagelist[k].Empty())
    {
        span* kspan = pagelist[k].PopFront();
        for (PAGE_ID i = 0; i < kspan->_pagenum; i++)
        {
            pagehash.set(kspan->_pageId + i, kspan);
        }

        return kspan;
    }

    for (size_t i = k + 1; i < PAGE_NUM; i++)
    {
        if (!pagelist[i].Empty())
        {
            span* spans = pagelist[i].PopFront();
            span* kspan = memory_pool.New();

            kspan->_pageId = spans->_pageId;
            kspan->_pagenum = k;

            spans->_pageId += k;
            spans->_pagenum -= k;
            

            pagelist[spans->_pagenum].PushFront(spans);

            pagehash.set(spans->_pageId, spans);
            pagehash.set(spans->_pageId + spans->_pagenum - 1, spans);

            for (PAGE_ID i = 0; i < kspan->_pagenum; i++)
            {
                pagehash.set(kspan->_pageId + i, kspan);
            }
             return kspan;
        } 
    }

    span* newspan = memory_pool.New();
    void* ptr = systemalloc(PAGE_NUM - 1);
    newspan->_pageId = (PAGE_ID)ptr >> PAGE_SIZE;
    newspan->_pagenum = PAGE_NUM - 1;
    pagelist[PAGE_NUM - 1].PushFront(newspan);

    return allocmemory_from_pagecache_to_centralcache(k);
}

span* PageCache::MapObjectToSpan(void* obj)
{
    PAGE_ID page_id = (PAGE_ID)obj >> PAGE_SIZE;
    assert(page_id);
    auto ret = (span*)pagehash.get(page_id);
    assert(ret);
    return ret;
}

void PageCache::ReleaseSpanToPageCache(span* free_span)
{
    assert(free_span);
    if (free_span->_pagenum > PAGE_NUM - 1)
    {
        void* ptr =(void *) pagehash.get(free_span->_pageId >> PAGE_SIZE);
        SystemFree(ptr);

        memory_pool.clear(free_span);
        return;
    }

    while (true)
    {
        auto prev=(span*)pagehash.get(free_span->_pageId - 1);
        if (prev == nullptr)
            break;
        span* prevspan= prev;
        if (prevspan->is_used)
            break;
        if (prevspan->_pagenum + free_span->_pagenum > PAGE_NUM-1)
            break;
        free_span->_pageId = prevspan->_pageId;
        free_span->_pagenum += prevspan->_pagenum;

        pagelist[prevspan->_pagenum].clearspan(prevspan);
        memory_pool.clear(prevspan);
    }

    while (true)
    {
        auto next = (span*)pagehash.get(free_span->_pageId+free_span->_pagenum);
        if (next == nullptr)
            break;
        span* nextspan = next;
        if (nextspan->is_used)
            break;
        if (nextspan->_pagenum + free_span->_pagenum > PAGE_NUM - 1)
            break;
        free_span->_pagenum += nextspan->_pagenum;

        pagelist[nextspan->_pagenum].clearspan(nextspan);
        memory_pool.clear(nextspan);
    }
    pagelist[free_span->_pagenum].PushFront(free_span);

    pagehash.set(free_span->_pageId, free_span);
    pagehash.set(free_span->_pageId + free_span->_pagenum, free_span);

    free_span->is_used = false;
}