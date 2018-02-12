#include "lru_cache.h"
#include "glog/logging.h"

template <typename K, typename V> Queue<K, V>::~Queue() {
  Node<K, V> *n = front_;
  while (n != nullptr) {
    Node<K, V> *d = n;
    n = n->next;
    delete d;
  }
}

template <typename K, typename V> const Node<K, V> *Queue<K, V>::Front() {
  if (front_ == nullptr) {
    return nullptr;
  }
  return front_;
}

template <typename K, typename V>
void Queue<K, V>::Push(const K &key, std::unique_ptr<V> data) {
  Node<K, V> *n = new Node<K, V>(key, std::move(data));
  if (front_ == end_ && front_ == nullptr) {
    front_ = end_ = n;
    ++size_;
    return;
  }

  // append n to end.
  end_->next = n;
  n->prev = end_;
  end_ = n;

  ++size_;
}

template <typename K, typename V> void Queue<K, V>::Pop() {
  if (end_ == nullptr && front_ == nullptr) {
    return;
  }

  if (front_ == end_) {
    delete front_;
    front_ = end_ = nullptr;
    --size_;
    return;
  }

  Node<K, V> *n = front_;
  front_->next->prev = nullptr;
  front_ = front_->next;
  delete n;

  --size_;
}

template <typename K, typename V> void Queue<K, V>::MoveToEnd(Node<K, V> *n) {
  if (n == nullptr) {
    return;
  }

  // case 1: n is the end node. no-op.
  if (n == end_) {
    return;
  }

  // case 2: n is the front. move to end.
  if (n == front_) {
    // remove from front.
    front_->next->prev = nullptr;
    front_ = front_->next;
    // append to end.
    end_->next = n;
    n->next = nullptr;
    n->prev = end_;
    end_ = n;
    return;
  }

  // case 3: neither front nor end. n is in the middle of the list.
  // a <-> n <-> b
  n->prev->next = n->next;
  n->next->prev = n->prev;
  // append to end.
  end_->next = n;
  n->next = nullptr;
  n->prev = end_;
  end_ = n;
  return;
}

///////////////////////////////////////////////////////////////////////////////
// LRUCache
template <typename K, typename V>
LRUCache<K, V>::LRUCache(int size) : capacity_(size) {}

// template <typename K, typename V>
// LRUCache<K, V>::LRUCache(int size, std::function<bool(K, V *)> f)
//     : capacity_(size), curSize_(0), evict_func_(f) {}

template <typename K, typename V>
bool LRUCache<K, V>::Put(const K &key, std::unique_ptr<V> value) {
  auto it = pool_.find(key);
  if (it != pool_.end()) {
    LOG(INFO) << "LRU PUT, exists...";
    // k-v pair already exists.
    return false;
  }

  // when buffer is full.
  if (static_cast<int>(pool_.size()) == capacity_) {
    LOG(INFO) << "LRU PUT, size full...";
    const Node<K, V> *victim = lru_queue_.Front();
    auto got = pool_.find(victim->key);
    if (got == pool_.end()) {
      LOG(FATAL) << "key not found in cache";
      return false;
    }

    if (evict_func_) {
      if (!evict_func_(got->first, got->second->data.get())) {
        return false;
      }
    }

    // Remove from the pool.
    pool_.erase(victim->key);
    lru_queue_.Pop();
  }

  // Insert value.
  LOG(INFO) << "LRU PUT, insert ...";
  lru_queue_.Push(key, std::move(value));
  auto res = pool_.insert(std::make_pair(key, lru_queue_.End()));
  if (!res.second) {
    LOG(ERROR) << "failed to insert key";
    return false;
  }

  return true;
}

template <typename K, typename V> V *LRUCache<K, V>::Get(const K &key) {
  auto res = pool_.find(key);
  if (res == pool_.end()) {
    return nullptr;
  }
  lru_queue_.MoveToEnd(res->second);
  return res->second->data.get();
}
