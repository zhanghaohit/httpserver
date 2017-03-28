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
#include "net.h"
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
  caches_.erase(path);
  return size;
}

int HttpResource::Get(const string& path, void* buf, int size) {
  string abs_path = root_+path;
  struct stat st;
  if (stat(abs_path.c_str(), &st)) {
      LOG(LOG_WARNING, "access %s failed: %s", abs_path.c_str(), strerror(errno));
      return 0;
  }

  if (S_ISDIR(st.st_mode)) { //directory (do not support list files function)
    abs_path += "index.html";
    if (stat(abs_path.c_str(), &st)) {
        LOG(LOG_WARNING, "access %s failed: %s", abs_path.c_str(), strerror(errno));
        return 0;
    }
  }

  if (caches_.count(abs_path)) { //cached
    std::pair<timespec, string>& data = caches_[abs_path];
    if (data.first < st.st_mtim) {
      LOG(LOG_WARNING, "found updated file");
      caches_.erase(abs_path);
    } else {
      int sz = data.second.length();
      if(sz > size) return sz;
      memcpy (buf, data.second.c_str(), sz);
      return sz;
    }
  }

  //normal process
  std::ifstream ifs(abs_path);
  int file_size = st.st_size;
  if (file_size > size) {
    LOG(LOG_WARNING, "file (%d) is larger than the buffer (%d)", file_size, size);
    return file_size;
  }
  ifs.read(static_cast<char*>(buf), file_size);
  ifs.close();
  caches_[abs_path] = std::make_pair(st.st_mtim, string(static_cast<char*>(buf), file_size));
  LOG(LOG_WARNING, "file_size = %d, file = %s", file_size, caches_[abs_path].second.c_str());
  return file_size;
}

} //end of namespace


