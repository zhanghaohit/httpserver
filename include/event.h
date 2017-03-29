/*
 * event.h
 *
 *  Created on: Mar 26, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_EVENT_H_
#define INCLUDE_EVENT_H_

#include <time.h>
#include <vector>

using std::vector;

namespace httpserver {
#define NONE 0
#define READABLE 1
#define WRITABLE 2
#define EDEGE 4

#define FILE_EVENTS 1
#define TIME_EVENTS 2
#define ALL_EVENTS (FILE_EVENTS|TIME_EVENTS)
#define DONT_WAIT 4

#define NOMORE -1

#define MAX_FD 10000

struct EventLoop;

// Types and data structures
typedef void FileProc(struct EventLoop *event_loop, int fd, void *client_data,
                        int mask);
typedef int TimeProc(struct EventLoop *event_loop, long long id,
                       void *client_data);
typedef void EventFinalizerProc(struct EventLoop *event_loop,
                                  void *client_data);

// File event structure
struct FileEvent {
  int mask; // one of AE_(READABLE|WRITABLE)
  FileProc *rfile_proc;
  FileProc *wfile_proc;
  void *client_data;
};

// Time event structure
struct TimeEvent {
  long long id; // time event identifier
  long when_sec; // seconds
  long when_ms; // milliseconds
  TimeProc *time_proc;
  EventFinalizerProc *finalizer_proc;
  void *client_data;
  TimeEvent *next;
};

// A fired event
struct FiredEvent {
  int fd;
  int mask;
};

struct EpollState {
  int epfd;
  struct epoll_event *events;
};

class EventLoop {
 public:
  EventLoop(int setsize = MAX_FD);
  ~EventLoop();

  void Start();
  void Stop();

  int CreateFileEvent(int fd, int mask, FileProc *proc, void *client_data);
  void DeleteFileEvent(int fd, int mask);
  int GetFileEvents(int fd);
  long long CreateTimeEvent(long long milliseconds, TimeProc *proc,
                              void *client_data,
                              EventFinalizerProc *finalizer_proc);
  int DeleteTimeEvent(long long id);

 private:
  int ResizeSetSize(int setsize);
  int ProcessEvents(int flags);
  int ProcessTimeEvents();
  TimeEvent* SearchNearestTimer();
  int EpollAddEvent(int fd, int mask);
  void EpollDelEvent(int fd, int delmask);
  int EpollPoll(struct timeval *tvp);

  int maxfd_; // highest file descriptor currently registered
  int setsize_; // max number of file descriptors tracked
  long long time_event_next_id_;
  time_t last_time_; // Used to detect system clock skew
  vector<FileEvent> events_; // Registered events
  vector<FiredEvent> fired_; // Fired events
  TimeEvent *time_event_head_;
  int stop_;
  void *apidata_; // This is used for polling API specific data
};
}

#endif
