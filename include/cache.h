/*
 * cache.h
 *
 *  Created on: Mar 29, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_CACHE_H_
#define INCLUDE_CACHE_H_

#include <string>
using std::string;
#include "lock.h"
#include "log.h"

namespace httpserver {

struct CacheEntry {
  string path;
  string data;
  timespec time;
};

class HttpCache {
 public:
  explicit HttpCache(int size);
  ~HttpCache();

  //Get is not thread-safe, users have to lock and unlock themselves
  //because we return the pointer rather that copy the data
  string* Get(const string& path, const timespec& nt = {0, 0});

  //Put and Delete are thread-safe since we can do it internally
  void Put(const string& path, const string& data, const timespec& time);
  void Put(const string& path, string&& data, const timespec& time);
  void Delete(const string& path);

  inline void Lock(const string& path) {
    size_t hash = hasher_(path) % size_;
    Lock(hash);
  }

  inline void UnLock(const string& path) {
    size_t hash = hasher_(path) % size_;
    UnLock(hash);
  }

  inline void Lock(size_t hash) {
    locks_[hash].lock();
  }

  inline void UnLock(size_t hash) {
    locks_[hash].unlock();
  }

 private:
  int size_;
  CacheEntry** entries_ = nullptr;
  Locker* locks_ = nullptr;
  std::hash<std::string> hasher_;
};

} //end of namespace


#endif /* INCLUDE_CACHE_H_ */
