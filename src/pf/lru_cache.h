#ifndef PF_PF_LRU_CACHE_H
#define PF_PF_LRU_CACHE_H

#include <functional>
#include <memory>
#include <unordered_map>

template <typename K, typename V> struct Node {
  K key;
  std::unique_ptr<V> data;

  Node *prev;
  Node *next;

  Node(K key, std::unique_ptr<V> data)
      : key(key), data(std::move(data)), prev(nullptr), next(nullptr) {}
};

template <typename K, typename V> class Queue {
public:
  Queue() : front_(nullptr), end_(nullptr), size_(0) {}
  ~Queue();
  Queue(const Queue &q) = delete;

  void Push(const K &key, std::unique_ptr<V> data);
  void Pop();
  const Node<K, V> *Front();
  void MoveToEnd(Node<K, V> *n);
  int Size() { return size_; }

  Node<K, V> *Begin() { return front_; }
  Node<K, V> *End() { return end_; }

private:
  Node<K, V> *front_;
  Node<K, V> *end_;

  int size_;
};

template <typename K, typename V> class LRUCache {
public:
  class Iterator {
  public:
    explicit Iterator(
        typename std::unordered_map<K, Node<K, V> *>::iterator begin,
        typename std::unordered_map<K, Node<K, V> *>::iterator end)
        : begin_(begin), end_(end), it_(begin) {}
    const K &Key() { return it_->first; }
    V *Data() { return it_->second->data.get(); }
    bool HasNext() { return it_ != end_; }
    void Next() { ++it_; }

  private:
    typename std::unordered_map<K, Node<K, V> *>::iterator begin_;
    typename std::unordered_map<K, Node<K, V> *>::iterator end_;
    typename std::unordered_map<K, Node<K, V> *>::iterator it_;
  };

  explicit LRUCache(int size);
  //  explicit LRUCache(int size, std::function<bool(K, V *)> f);
  ~LRUCache() = default;
  void SetEvictFunction(std::function<bool(K, V *)> f) { evict_func_ = f; }
  bool Put(const K &key, std::unique_ptr<V> page);
  V *Get(const K &key);

  bool Empty() { return curSize_ == 0; }
  int Size() { return curSize_; }
  int Capacity() { return capacity_; }

  Iterator GetIterator() { return Iterator(pool_.begin(), pool_.end()); }

private:
  int capacity_;
  int curSize_;
  Queue<K, V> lru_queue_;
  std::unordered_map<K, Node<K, V> *> pool_;
  std::function<bool(K, V *)> evict_func_;
};

// C++ requires template to have its declaration be in the same file as its
// definition.
#include "lru_cache.cc"

#endif // PF_PF_LRU_CACHE
