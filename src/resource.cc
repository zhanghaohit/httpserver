/*
 * resource.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <fstream>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "resource.h"
#include "log.h"
#include "util.h"

namespace httpserver {

HttpResource* HttpResource::resource_ = nullptr;

int HttpResource::Put(const string& path, const void* data, int size) {
  string tmp_path = ".tmp_" + path;
  std::ofstream ofs(tmp_path);
  if(!ofs.is_open()) {
    LOG(LOG_WARNING, "%s cannot be opened", path.c_str());
    return 0;
  }
  ofs.write(static_cast<const char*>(data), size);
  ofs.close();

  if (rename (tmp_path.c_str(), path.c_str())) {
    LOG(LOG_WARNING, "cannot write to %s: %s", path.c_str(), strerror(errno));
    return 0;
  }
#ifdef USE_CACHE
  caches_.Delete(path);
#endif
  return size;
}

int HttpResource::Get(const string& path, void* buf, int size) {
  string abs_path = root_+path;
  struct stat st;
  if (stat(abs_path.c_str(), &st)) {
      LOG(LOG_INFO, "access %s failed: %s", abs_path.c_str(), strerror(errno));
      return 0;
  }

  /*
   * path is a directory
   * TODO: if index.html not exists, list the files in the directory
   */
  if (S_ISDIR(st.st_mode)) {
    abs_path += "/index.html";
    if (stat(abs_path.c_str(), &st)) {
        LOG(LOG_WARNING, "access %s failed: %s", abs_path.c_str(), strerror(errno));
        return 0;
    }
  }

#ifdef USE_CACHE
  /*
   * for cache.Get(), users are required to explicitly lock/unlock
   * because we allow the users to keep the data for a while
   * in order to avoid an extra copy
   */
  caches_.Lock(abs_path);
  string* data = caches_.Get(abs_path, st.st_mtim);
  if (data) { //cached
    int sz = data->length();
    if (sz > size) {
      caches_.UnLock(abs_path);
      return sz;
    }
    memcpy(buf, data->c_str(), sz);
    caches_.UnLock(abs_path);
    return sz;
  }
  caches_.UnLock(abs_path);
#endif

  //normal process
  std::ifstream ifs(abs_path);
  int file_size = st.st_size;
  if (file_size > size) {
    LOG(LOG_INFO, "file (%d) is larger than the buffer (%d)", file_size, size);
    return file_size;
  }
  ifs.read(static_cast<char*>(buf), file_size);
  ifs.close();
#ifdef USE_CACHE
  caches_.Put(abs_path, string(static_cast<char*>(buf), file_size), st.st_mtim);
#endif
  return file_size;
}

} //end of namespace


