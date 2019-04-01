#pragma once

#include <base/noncopyable.h>

#include <stdint.h>

template<typename T>
class AtomicIntegerT : noncopyable {
public:
	AtomicIntegerT() : value_(0) {}

	T get() { return __atomic_load_n(&value_, __ATOMIC_SEQ_CST); }
	T getAndAdd(T x) { return __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST); }
	T addAndGet(T x) { return getAndAdd(x) + x;	}

	T increAndGet() { return addAndGet(1);	}
	T decreAndGet(){ return addAndGet(-1); }

	void add(T x)	{ getAndAdd(x);	}

	void incre(){ increAndGet();}
	void decre(){ decreAndGet();}

	T getAndSet(T newValue)	{ return __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST);}

private:
	volatile T value_;
};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

