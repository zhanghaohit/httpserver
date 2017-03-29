/*
 * resource.h
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_RESOURCE_H_
#define INCLUDE_RESOURCE_H_

#include <unordered_map>
#include <string>
#include <ctime>
#include "cache.h"
#include "settings.h"

using std::string;
using std::unordered_map;

namespace httpserver {
class HttpResource {
 public:

  static HttpResource* Instance() {
    if (!resource_) {
      resource_ = new HttpResource();
    }
    return resource_;
  }

  inline void SetRootDir (const string& root) {
    root_ = root;
  }

  inline string GetRootDir () const noexcept {
    return root_;
  }

  //currently this function is not used as we do not support writing something in the server
  //TODO: thread-safe
  int Put (const string& path, const void* data, int size);

  int Get (const string& path, void* buf, int size);
  string Get (const string& path);

  HttpResource(const HttpResource&) = delete;
  void operator=(const HttpResource&) = delete;

 private:
  HttpResource() {};
  static HttpResource* resource_;
  string root_ = "./";
#ifdef USE_CACHE
  HttpCache caches_ {DEFAULT_CACHE_SIZE};
#endif
};
} //end of namespace


#endif /* INCLUDE_RESOURCE_H_ */
