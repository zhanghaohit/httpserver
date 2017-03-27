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
#include "resource.h"
#include "log.h"

namespace httpserver {

int HttpResource::Put(const string& path, const void* data, int size) {
  string tmp_path = ".tmp_" + path;
  std::ofstream ofs(tmp_path);
  if(!ofs.is_open()) {
    LOG(LOG_WARNING, "%s cannot be opened", path.c_str());
    return 0;
  }
  ofs.write(static_cast<char*>(data), size);
  ofs.close();

  if (rename (tmp_path.c_str(), path.c_str())) {
    LOG(LOG_WARNING, "cannot write to %s: %s", path.c_str(), strerror(errno));
    return 0;
  }
  caches_.erase(path);
  return size;
}

int HttpResource::Get(const string& path, void* buf, int size) {
  if (caches_.count(path)) { //cached
    const string& data = caches_[path];
    int sz = data.length() > size ? size : data.length();
    memcpy (buf, data.c_str(), sz);
    return size;
  }

  //normal process
  std::ifstream ifs(path, std::ifstream::ate | std::ifstream::binary);
  int file_size = ifs.tellg();
  if (file_size > size) {
    LOG(LOG_WARNING, "file (%d) is larger than the buffer (%d)", file_size, size);
    return 0;
  }
  ifs.seekg (0, ifs.beg);
  ifs.read(static_cast<char*>(buf), file_size);
  return file_size;
}

} //end of namespace


