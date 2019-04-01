#include <base/Atomic.h>
#include <assert.h>

int main(){

  {
  AtomicInt64 a0;
  assert(a0.get() == 0);
  assert(a0.getAndAdd(1) == 0);
  assert(a0.get() == 1);
  assert(a0.addAndGet(2) == 3);
  assert(a0.get() == 3);
  assert(a0.increAndGet() == 4);
  assert(a0.get() == 4);
  a0.incre();
  assert(a0.get() == 5);
  assert(a0.addAndGet(-3) == 2);
  assert(a0.getAndSet(100) == 2);
  assert(a0.get() == 100);
  }

  {
  AtomicInt32 a1;
  assert(a1.get() == 0);
  assert(a1.getAndAdd(1) == 0);
  assert(a1.get() == 1);
  assert(a1.addAndGet(2) == 3);
  assert(a1.get() == 3);
  assert(a1.increAndGet() == 4);
  assert(a1.get() == 4);
  a1.incre();
  assert(a1.get() == 5);
  assert(a1.addAndGet(-3) == 2);
  assert(a1.getAndSet(100) == 2);
  assert(a1.get() == 100);
  }
}

