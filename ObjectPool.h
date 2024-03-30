#pragma once
#include"Common.h"

template<class T>
class Objectpool
{
private:
	int _freespacebytes = 0;
	void* _freelist = nullptr;
	char* _memory=nullptr;
public:
	T* New()
	{
		T* obj = nullptr;
		if(_freelist)
		{
			obj = (T*)_freelist;
			_freelist = NextObj(_freelist);
		}
        else
        {   
			if (_freespacebytes < sizeof(T))
			{
				_freespacebytes = 128 * 1024;
				_memory = (char*)systemalloc(_freespacebytes>>PAGE_SIZE);
				if (_memory == nullptr)
					throw std::bad_alloc();
			}
			obj = (T*)_memory;
			int objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_freespacebytes -= objSize;
        }

		new(obj)T;
		return obj;
	}

	void clear(T* obj)
	{
		obj->~T();
		NextObj(obj)=_freelist;
		_freelist = obj;
	}
};