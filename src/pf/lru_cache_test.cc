#include <string>

#include "glog/logging.h"
#include "googletest/include/gtest/gtest.h"
#include "lru_cache.h"

namespace {
TEST(Queue, PushPop) {
  Queue<std::string, int> q;
  EXPECT_EQ(q.Size(), 0);

  std::string str[3] = {"a", "b", "c"};

  // Push
  for (int i = 0; i < 3; i++) {
    q.Push(str[i], std::unique_ptr<int>(new int(i)));
    EXPECT_EQ(q.Size(), i + 1);
    LOG(INFO) << "Push ok " << i;
  }

  // Pop and verify via Front
  for (int i = 0; i < 3; i++) {
    const Node<std::string, int> *got = q.Front();
    EXPECT_EQ(got->key, str[i]);
    EXPECT_EQ(*(got->data), i);
    LOG(INFO) << "Front ok " << i;
    q.Pop();
    EXPECT_EQ(q.Size(), 3 - i - 1);
    LOG(INFO) << "Pop ok " << i;
  }

  EXPECT_EQ(q.Size(), 0);
  LOG(INFO) << "delete ...";
}

TEST(Queue, MoveToEnd) {
  Queue<std::string, int> q;
  EXPECT_EQ(q.Size(), 0);

  std::string str[3] = {"a", "b", "c"};

  // Push and build a queue: {"a", 1} <--> {"b", 2} <--> {"c", 3}
  for (int i = 0; i < 3; i++) {
    q.Push(str[i], std::unique_ptr<int>(new int(i)));
    EXPECT_EQ(q.Size(), i + 1);
    LOG(INFO) << "Push ok " << i;
  }

  // Move end to end is a no-op.
  {
    auto *n = q.End();
    q.MoveToEnd(n);
    EXPECT_EQ(q.Front()->key, str[0]);
    EXPECT_EQ(*(q.Front()->data), 0);
  }

  // Move front to end;
  {
    for (int i = 0; i < 3; i++) {
      EXPECT_EQ(q.Front()->key, str[i]);
      EXPECT_EQ(*(q.Front()->data), i);
      LOG(INFO) << "Front ok " << i;
      auto *n = q.Begin();
      q.MoveToEnd(n);
      EXPECT_EQ(q.Front()->key, str[(i + 1) % 3]);
      EXPECT_EQ(*(q.Front()->data), (i + 1) % 3);
    }
  }

  // now queue is back to {"a", 1} <--> {"b", 2} <--> {"c", 3}
  {
    auto *n = q.End();
    q.Push("new_end", std::unique_ptr<int>(new int(100)));
    EXPECT_EQ(q.Size(), 4);
    q.MoveToEnd(n);
    auto *got = q.End();
    EXPECT_EQ(got->key, "c");
    EXPECT_EQ(*(got->data), 2);
  }
}
} // namespace

namespace {
TEST(LRUCache, Put) {
  const int cap = 3;
  LRUCache<std::string, int> cache(cap);
  EXPECT_EQ(cache.Capacity(), cap);
  LOG(INFO) << "Capacity ok";

  // insert element until cache is full.
  std::string str[3] = {"a", "b", "c"};
  for (int i = 0; i < cap; i++) {
    LOG(INFO) << "Put ok " << i;
    EXPECT_TRUE(cache.Put(str[i], std::unique_ptr<int>(new int(i))));
  }

  for (int i = 0; i < cap; i++) {
    LOG(INFO) << "Get ok " << i;
    int *got = cache.Get(str[i]);
    EXPECT_EQ(*got, i);
  }
}
} // namespace
