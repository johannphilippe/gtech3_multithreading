#pragma once

#include<Windows.h>
#include<cstdio>
#include<processthreadsapi.h>

template<typename T>
class SpinQueue {
public:
	SpinQueue(size_t size)
		: _size(0)
		, _capacity(size)
		, _data(NULL)
		, _lock(0)
		, _resizing(0)
		, _first_out_index(0)

	{
		_data = (T*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, 
			sizeof(T) * _capacity);
		
		if (_data == NULL) {
			printf("HeapAlloc failed in SafeQueue \n");
		}
	}

	~SpinQueue() {
		HeapFree(GetProcessHeap(), 0, _data);
	}

	bool push(const T& item, DWORD timeout_ms = 20) {
		LONG64 current_size = InterlockedCompareExchange64(&_size, 0, 0);
		LONG64 current_capacity = InterlockedCompareExchange64(&_capacity, 0, 0);
		if (current_size >= current_capacity) {
			if (!_realloc(current_capacity * 2)) {
				return false;
			}
		}
		if (!_wait_lock(timeout_ms, &_lock)) {
			return false;
		}
		current_size = InterlockedCompareExchange64(&_size, 0, 0);
		current_capacity = InterlockedCompareExchange64(&_capacity, 0, 0);
		if (current_size < current_capacity) {
			size_t write_index = (_first_out_index + current_size) % current_capacity;
			_data[write_index] = item;
			//_size++;
			InterlockedIncrement64(&_size );
			_unlock(&_lock);
		}
		else { // Case another thread resized the queue
			_unlock(&_lock);
			return push(item, timeout_ms);
		}

		return true;
	}

	bool try_pop(T& item, DWORD timeout_ms = 20) {
		if (!_wait_lock(timeout_ms, &_lock)) {
			return false;
		}
		LONG64 current_size = InterlockedCompareExchange64(&_size, 0, 0);
		if (current_size <= 0) {
			//printf("Queue is empty in pop\n");
			_unlock(&_lock);
			return false;
		}

		item = _data[_first_out_index];
		InterlockedDecrement64(&_size);
		//_size--, 
		_first_out_index++;
		LONG64 current_capacity = InterlockedCompareExchange64(&_capacity, 0, 0);
		if(_first_out_index >= current_capacity){
			_first_out_index = 0;
		}

		_unlock(&_lock);
		return true;
	}

private:



	bool _realloc(size_t s, DWORD timeout_ms = 20) {
		if (!_wait_lock(20, &_resizing)) {
			return false;
		}

		T* new_mem = (T*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS,
			sizeof(T) * s);
		
		
			if (new_mem == NULL) {
			InterlockedExchange64(&_resizing, 0);
			return false;
		}

		if (!_wait_lock(timeout_ms, &_lock)) {
			HeapFree(GetProcessHeap(), 0 , new_mem );
			InterlockedExchange64(&_resizing, 0);
			return false;
		}

		LONG64 current_size = InterlockedCompareExchange64(&_size, 0, 0);
		LONG64 current_capacity = InterlockedCompareExchange64(&_capacity, 0, 0);
		size_t copy_size = (current_size < s) ? current_size : s;
		if(_first_out_index + copy_size <= current_capacity){
			CopyMemory(new_mem, _data + _first_out_index, sizeof(T) * copy_size);
		}
		else {
			size_t first_part_size = current_capacity - _first_out_index;
			CopyMemory(new_mem, _data + _first_out_index, sizeof(T) * first_part_size);
			CopyMemory(new_mem + first_part_size, _data, sizeof(T) * (copy_size - first_part_size));
		}
		_first_out_index = 0;

		T* tmp = _data;
		_data = new_mem;
		InterlockedCompareExchange64(&_capacity, s, current_capacity);
		//_capacity = s;
		_unlock(&_lock);
		BOOL free_res = HeapFree(GetProcessHeap(), 0, tmp);
		if (free_res == FALSE) {
			printf("HeapFree failed in SafeQueue _realloc\n");
		}
		_unlock(&_resizing);
		return true;
	}

	bool _try_lock(LONG64* lock) {
		return InterlockedCompareExchange64(lock, 1, 0) == 0;
	}

	bool _wait_lock(DWORD timeout_ms, LONG64* lock) {
		DWORD start_time = GetTickCount64();
		size_t spins = 0;
		while (true) {
			if (_try_lock(lock)) { return true; }

			if (GetTickCount64() - start_time > timeout_ms) {
				return false;
			}

			if (spins < 8) {
				YieldProcessor();
			}
			else if (spins < 16) {
				SwitchToThread();
			}
			else if (spins < 32) {
				Sleep(0);
			}
			else {
				Sleep(1);
			}

			spins++;
		}
		return true;
	}

	void _unlock(LONG64 *lock) {
		InterlockedExchange64(lock, 0);
	}

	LONG64 _size;
	LONG64 _capacity;

	T* _data;
	LONG64 _lock, _resizing;

	LONG64 _first_out_index; 
};

template<typename T>
class LockQueue {
public:

	LockQueue(LONG64 capacity)
		: _size(0)
		, _capacity(capacity)
		, _data(NULL)
		, _lock()
		, _resizing(0)
		, _first_out_index(0)
	{
		_data = (T*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS,
			sizeof(T) * _capacity);
		if (_data == NULL) {
			printf("HeapAlloc failed in LockQueue \n");
		}
		_lock = (LPCRITICAL_SECTION)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS,
			sizeof(CRITICAL_SECTION));
		if (_lock == NULL) {
			printf("HeapAlloc failed for lock in LockQueue \n");
			RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, 
				EXCEPTION_NONCONTINUABLE, NULL, NULL);
		}
		InitializeCriticalSection(_lock);
	}

	bool push(const T& item, size_t timeout_ms = 20)
	{
		EnterCriticalSection(_lock);
		if (_size >= _capacity) {
			if (!_realloc(_capacity * 2)) {
				LeaveCriticalSection(_lock);
				printf("Realloc failed in LockQueue push\n");
				return false; 
			}
		}
		size_t write_index = (_first_out_index + _size) % _capacity;
		_data[write_index] = item;
		_size++;
		LeaveCriticalSection(_lock);
		return true;
	}

	bool pop(T& item, size_t timeout_ms = 20)
	{
		EnterCriticalSection(_lock);
		if (_size <= 0) {
			LeaveCriticalSection(_lock);
			return false;
		}

		item = _data[_first_out_index];
		_size--, _first_out_index++;
		if (_first_out_index >= _capacity) {
			_first_out_index = 0;
		}
		LeaveCriticalSection(_lock);
		return true;
	}

private:


	bool _realloc(LONG64 s)
	{
		T* new_mem = (T*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS,
			sizeof(T) * s);

		if (new_mem == NULL) {
			printf("HeapAlloc failed in LockQueue _realloc \n");
			return false;
		}
		size_t copy_size = (_size < s) ? _size : s;
		if (_first_out_index + copy_size <= _capacity) {
			CopyMemory(new_mem, _data + _first_out_index, sizeof(T) * copy_size);
		}
		else {
			size_t first_part_size = _capacity - _first_out_index;
			CopyMemory(new_mem, _data + _first_out_index, sizeof(T) * first_part_size);
			CopyMemory(new_mem + first_part_size, _data, sizeof(T) * (copy_size - first_part_size));
		}
		_first_out_index = 0;
		T* tmp = _data;
		_data = new_mem;
		_capacity = s;
		
		BOOL free_res = HeapFree(GetProcessHeap(), 0, tmp);
		if (free_res == FALSE) {
			printf("HeapFree failed in LockQueue _realloc\n");
		}
		return true;
	}



	LONG64 _size;
	LONG64 _capacity;

	T* _data;
	LPCRITICAL_SECTION _lock;
	LONG64 _resizing;
	LONG64 _first_out_index;
};