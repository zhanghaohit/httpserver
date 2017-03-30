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

namespace httpserver {

/*
 * used to maintain the website resources (i.e., files, http caches)
 */
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

  /*
   * put a new file to the directory
   * currently this function is not used as we do not support writing something in the server
   * TODO: thread-safe
   */
  int Put(const string& path, const void* data, int size);

  /*
   * read the data from path and put it in the buffer
   * @path: the file location
   * @buf: user-provied buffer
   * @size: max size
   * return: the file size
   */
  int Get(const string& path, void* buf, int size);

  //disable copy constructor
  HttpResource(const HttpResource&) = delete;
  void operator=(const HttpResource&) = delete;

 private:
  HttpResource() {};
  static HttpResource* resource_; //singleton instance
  string root_ = "./";
#ifdef USE_CACHE
  HttpCache caches_ {DEFAULT_CACHE_SIZE}; //caches used to cache some recently accessed files
#endif
};
} //end of namespace


#endif /* INCLUDE_RESOURCE_H_ */
