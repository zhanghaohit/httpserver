/*
 * cache.cc
 *
 *  Created on: Mar 29, 2017
 *      Author: zhanghao
 */

#include <string.h>
#include "cache.h"
#include "util.h"

namespace httpserver {

HttpCache::HttpCache(int size): size_(size) {
  entries_ = new CacheEntry*[size_];
  memset(entries_, 0, sizeof(CacheEntry*)*size_);
  locks_ = new Locker[size_];
}

HttpCache::~HttpCache() {
  for (int i = 0; i < size_; i++) {
    if (entries_[i]) delete entries_[i];
  }
  delete entries_;
  delete locks_;
}

string* HttpCache::Get(const string& path, const timespec& nt) {
  size_t hash = hasher_(path) % size_;

  if (entries_[hash]) {
    if (entries_[hash]->time < nt) {
      return nullptr;
    } else {
      return &entries_[hash]->data;
    }
  } else {
    return nullptr;
  }
}

void HttpCache::Put(const string& path, const string& data, const timespec& time) {
  size_t hash = hasher_(path) % size_;
  Lock(hash);
  if (entries_[hash]) {
    delete entries_[hash];
  }
  entries_[hash] = new CacheEntry{path, data, time};
  UnLock(hash);
}

void HttpCache::Put(const string& path, string&& data, const timespec& time) {
  size_t hash = hasher_(path) % size_;
  Lock(hash);
  if (entries_[hash]) {
    delete entries_[hash];
  }
  entries_[hash] = new CacheEntry{path, std::move(data), time};
  UnLock(hash);
}

void HttpCache::Delete(const string& path) {
  size_t hash = hasher_(path) % size_;
  Lock(hash);
  if (entries_[hash]) delete entries_[hash];
  entries_[hash] = nullptr;
  UnLock(hash);
}

} //end of namespace


