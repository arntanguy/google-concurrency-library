// This file is a rudimentary test of the thread class.  It primarily
// serves to make sure the build system works.

#include "thread.h"
#include "mutex.h"

#include "gmock/gmock.h"
#include <tr1/functional>

namespace tr1 = std::tr1;

void WaitForThenSet(mutex& mu, bool* ready, bool* signal) {
  lock_guard<mutex> l(mu);
  while (!*ready) {
    mu.unlock();
    this_thread::sleep_for(chrono::milliseconds(10));
    mu.lock();
  }
  *signal = true;
}

// Sets 'id' to the id of this thread.
void SetThreadId(thread::id* id) {
  *id = this_thread::get_id();
}

TEST(ThreadTest, StartsNewThread) {
  bool ready = false;
  bool signal = false;
  mutex mu;
  thread thr(tr1::bind(WaitForThenSet, tr1::ref(mu), &ready, &signal));
  lock_guard<mutex> l(mu);
  EXPECT_FALSE(signal);
  ready = true;
  while (!signal) {
    mu.unlock();
    this_thread::sleep_for(chrono::milliseconds(10));
    mu.lock();
  }
  thr.detach();
}

TEST(ThreadTest, JoinSynchronizes) {
  bool ready = true;
  bool signal = false;
  mutex mu;
  thread thr(tr1::bind(WaitForThenSet, tr1::ref(mu), &ready, &signal));
  thr.join();
  EXPECT_TRUE(signal);
}

TEST(ThreadTest, GetId) {
  thread::id id_set_by_thread;
  thread::id this_thread_id = this_thread::get_id();
  thread thr(tr1::bind(SetThreadId, &id_set_by_thread));
  thread::id id_of_thread = thr.get_id();
  thr.join();
  // The id that was set in SetThreadId() should equal the id of the
  // thread we created, and should not equal the current thread id.
  EXPECT_TRUE(id_set_by_thread == id_of_thread);
  EXPECT_FALSE(id_set_by_thread == this_thread_id);

  thread::id greater, lesser;
  if (this_thread_id < id_of_thread) {
    lesser = this_thread_id;
    greater = id_of_thread;
  } else {
    lesser = id_of_thread;
    greater = this_thread_id;
  }
  EXPECT_TRUE(greater > lesser);
  EXPECT_TRUE(lesser < greater);

  EXPECT_TRUE(greater >= lesser);
  EXPECT_FALSE(lesser >= greater);

  EXPECT_TRUE(lesser <= greater);
  EXPECT_FALSE(greater <= lesser);

  EXPECT_TRUE(lesser >= lesser);
  EXPECT_TRUE(lesser <= lesser);

  // The null/empty thread id should not equal the id of any real
  // thread.
  thread::id null_id;
  EXPECT_NE(id_set_by_thread, null_id);
  EXPECT_NE(this_thread_id, null_id);

  // After a join, a thread should no longer have an id.
  EXPECT_TRUE(null_id == thr.get_id());

  // Verify behaviour of null id in comparisons
  EXPECT_TRUE(null_id == null_id);
  EXPECT_FALSE(null_id == id_set_by_thread);

  EXPECT_TRUE(null_id < this_thread_id);
  EXPECT_FALSE(this_thread_id < null_id);

  EXPECT_TRUE(this_thread_id > null_id);
  EXPECT_FALSE(null_id > this_thread_id);

  EXPECT_TRUE(null_id <= this_thread_id);
  EXPECT_FALSE(this_thread_id <= null_id);

  EXPECT_TRUE(this_thread_id >= null_id);
  EXPECT_FALSE(null_id >= this_thread_id);

  EXPECT_FALSE(null_id < null_id);
  EXPECT_TRUE(null_id <= null_id);
}
