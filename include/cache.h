/*
 * cache.h
 *
 *  Created on: Mar 29, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_CACHE_H_
#define INCLUDE_CACHE_H_

#include <string>
#include "lock.h"

using std::string;

namespace httpserver {

struct CacheEntry {
  string path;
  string data;
  timespec time;
};

/*
 * simple http cache
 * fixed size cache
 * if two keys collide, evict the old one and insert the new one
 */
class HttpCache {
 public:
  explicit HttpCache(int size);
  ~HttpCache();

  /*
   * Get is not thread-safe, users have to lock and unlock themselves
   * because we return the pointer rather that copy the data
   * @path: as the cache key
   * @nt: current file modification time (if it is newer, which means cache is stale)
   */
  string* Get(const string& path, const timespec& nt = {0, 0});

  /*
   * Put and Delete are thread-safe since we can do it internally
   * @path: as the cache key
   * @nt: latest file modification time
   */
  void Put(const string& path, const string& data, const timespec& time);
  void Put(const string& path, string&& data, const timespec& time);
  void Delete(const string& path);

  //lock the cache entry based on the path
  inline void Lock(const string& path) {
    size_t hash = hasher_(path) % size_;
    Lock(hash);
  }

  //unlock the cache entry based on the path
  inline void UnLock(const string& path) {
    size_t hash = hasher_(path) % size_;
    UnLock(hash);
  }

 private:
  inline void Lock(size_t hash) {
    locks_[hash].lock();
  }

  inline void UnLock(size_t hash) {
    locks_[hash].unlock();
  }

  int size_; //cache size
  CacheEntry** entries_ = nullptr; //cache lines
  Locker* locks_ = nullptr; //locks used to protect cache lines

  /*
   * a standard hashing function
   * TODO: better hash function
   */
  std::hash<std::string> hasher_;
};

} //end of namespace

#endif /* INCLUDE_CACHE_H_ */
