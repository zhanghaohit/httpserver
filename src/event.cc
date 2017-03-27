#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/epoll.h>
#include "net.h"
#include "event.h"
#include "log.h"

namespace httpserver {

static void GetTime(long *seconds, long *milliseconds) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  *seconds = tv.tv_sec;
  *milliseconds = tv.tv_usec / 1000;
}

static void AddMillisecondsToNow(long long milliseconds, long *sec,
                                   long *ms) {
  long cur_sec, cur_ms, when_sec, when_ms;

  GetTime(&cur_sec, &cur_ms);
  when_sec = cur_sec + milliseconds / 1000;
  when_ms = cur_ms + milliseconds % 1000;
  if (when_ms >= 1000) {
    when_sec++;
    when_ms -= 1000;
  }
  *sec = when_sec;
  *ms = when_ms;
}

int EventLoop::EpollAddEvent(int fd, int mask) {
  EpollState *state = (EpollState *) apidata_;
  struct epoll_event ee;
  /* If the fd was already monitored for some event, we need a MOD
   * operation. Otherwise we need an ADD operation. */
  int op = events_[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  ee.events = 0;
  mask |= events_[fd].mask; /* Merge old events */
  if (mask & AE_READABLE)
    ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE)
    ee.events |= EPOLLOUT;
  if (mask & AE_EDEGE)
    ee.events |= EPOLLET;
  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;
  if (epoll_ctl(state->epfd, op, fd, &ee) == -1)
    return -1;
  return 0;
}

void EventLoop::EpollDelEvent(int fd, int delmask) {
  EpollState *state = (EpollState *) apidata_;
  struct epoll_event ee;
  int mask = events_[fd].mask & (~delmask);

  ee.events = 0;
  if (mask & AE_READABLE)
    ee.events |= EPOLLIN;
  if (mask & AE_WRITABLE)
    ee.events |= EPOLLOUT;
  ee.data.u64 = 0; /* avoid valgrind warning */
  ee.data.fd = fd;
  if (mask != AE_NONE) {
    epoll_ctl(state->epfd, EPOLL_CTL_MOD, fd, &ee);
  } else {
    /* Note, Kernel < 2.6.9 requires a non null event pointer even for
     * EPOLL_CTL_DEL. */
    epoll_ctl(state->epfd, EPOLL_CTL_DEL, fd, &ee);
  }
}

int EventLoop::EpollPoll(struct timeval *tvp) {
  EpollState *state = (EpollState *) apidata_;
  int retval, numevents = 0;

  retval = epoll_wait(state->epfd, state->events, setsize_,
                      tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);
  if (retval > 0) {
    int j;

    numevents = retval;
    for (j = 0; j < numevents; j++) {
      int mask = 0;
      struct epoll_event *e = state->events + j;
      if (e->events & EPOLLIN)
        mask |= AE_READABLE;
      if (e->events & EPOLLOUT)
        mask |= AE_WRITABLE;
      if (e->events & EPOLLERR)
        mask |= AE_WRITABLE;
      if (e->events & EPOLLHUP)
        mask |= AE_WRITABLE;
      fired_[j].fd = e->data.fd;
      fired_[j].mask = mask;
    }
  }
  return numevents;
}

EventLoop::EventLoop(int setsize) {
  int i;

  this->events_ = (FileEvent *) malloc(sizeof(FileEvent) * setsize);
  this->fired_ = (FiredEvent *) malloc(sizeof(FiredEvent) * setsize);
  if (this->events_ == NULL || this->fired_ == NULL)
    return;
  this->setsize_ = setsize;
  this->last_time_ = time(NULL);
  this->time_event_head_ = NULL;
  this->time_event_next_id_ = 0;
  this->stop_ = 0;
  this->maxfd_ = -1;
  /* Events with mask == AE_NONE are not set. So let's initialize the
   * vector with it. */
  for (i = 0; i < setsize; i++)
    this->events_[i].mask = AE_NONE;

  EpollState *state = (EpollState*) malloc(sizeof(EpollState));
  if (!state)
    return;
  state->events = (struct epoll_event*) malloc(
      sizeof(struct epoll_event) * this->setsize_);
  if (!state->events) {
    free(state);
    return;
  }
  state->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
  if (state->epfd == -1) {
    free(state->events);
    free(state);
    return;
  }
  this->apidata_ = state;
}

/* Resize the maximum set size of the event loop.
 * If the requested set size is smaller than the current set size, but
 * there is already a file descriptor in use that is >= the requested
 * set size minus one, AE_ERR is returned and the operation is not
 * performed at all.
 *
 * Otherwise AE_OK is returned and the operation is successful. */
int EventLoop::ResizeSetSize(int setsize) {
  int i;

  if (setsize == this->setsize_)
    return ST_SUCCESS;
  if (this->maxfd_ >= setsize)
    return ST_ERROR;

  EpollState *state = static_cast<EpollState*>(this->apidata_);
  state->events = (struct epoll_event*) realloc(
      state->events, sizeof(struct epoll_event) * setsize);

  this->events_ = (FileEvent *) realloc(this->events_,
                                         sizeof(FileEvent) * setsize);
  this->fired_ = (FiredEvent *) realloc(this->fired_,
                                         sizeof(FiredEvent) * setsize);
  this->setsize_ = setsize;

  /* Make sure that if we created new slots, they are initialized with
   * an AE_NONE mask. */
  for (i = this->maxfd_ + 1; i < setsize; i++)
    this->events_[i].mask = AE_NONE;
  return ST_SUCCESS;
}

EventLoop::~EventLoop() {
  EpollState *state = (EpollState *) apidata_;
  close(state->epfd);
  free(state->events);
  free(state);

  free(this->events_);
  free(this->fired_);
}

void EventLoop::Stop() {
  this->stop_ = 1;
}

int EventLoop::CreateFileEvent(int fd, int mask, FileProc *proc,
                                   void *client_data) {
  if (fd >= this->setsize_) {
    ResizeSetSize(this->setsize_ * 2);
  }

  if (fd >= this->setsize_) {
    errno = ERANGE;
    return ST_ERROR;
  }
  FileEvent *fe = &this->events_[fd];

  if (EpollAddEvent(fd, mask) == -1)
    return ST_ERROR;
  fe->mask |= mask;
  if (mask & AE_READABLE)
    fe->rfile_proc = proc;
  if (mask & AE_WRITABLE)
    fe->wfile_proc = proc;
  fe->client_data = client_data;
  if (fd > this->maxfd_)
    this->maxfd_ = fd;
  return ST_SUCCESS;
}

void EventLoop::DeleteFileEvent(int fd, int mask) {
  if (fd >= this->setsize_)
    return;
  FileEvent *fe = &this->events_[fd];
  if (fe->mask == AE_NONE)
    return;

  EpollDelEvent(fd, mask);
  fe->mask = fe->mask & (~mask);
  if (fd == this->maxfd_ && fe->mask == AE_NONE) {
    /* Update the max fd */
    int j;

    for (j = this->maxfd_ - 1; j >= 0; j--)
      if (this->events_[j].mask != AE_NONE)
        break;
    this->maxfd_ = j;
  }
}

int EventLoop::GetFileEvents(int fd) {
  if (fd >= this->setsize_)
    return 0;
  FileEvent *fe = &this->events_[fd];

  return fe->mask;
}

long long EventLoop::CreateTimeEvent(long long milliseconds,
                                         TimeProc *proc, void *client_data,
                                         EventFinalizerProc *finalizer_proc) {
  long long id = this->time_event_next_id_++;
  TimeEvent *te;

  te = (TimeEvent *) malloc(sizeof(*te));
  if (te == NULL)
    return ST_ERROR;
  te->id = id;
  AddMillisecondsToNow(milliseconds, &te->when_sec, &te->when_ms);
  te->time_proc = proc;
  te->finalizer_proc = finalizer_proc;
  te->client_data = client_data;
  te->next = this->time_event_head_;
  this->time_event_head_ = te;
  return id;
}

int EventLoop::DeleteTimeEvent(long long id) {
  TimeEvent *te, *prev = NULL;

  te = this->time_event_head_;
  while (te) {
    if (te->id == id) {
      if (prev == NULL)
        this->time_event_head_ = te->next;
      else
        prev->next = te->next;
      if (te->finalizer_proc)
        te->finalizer_proc(this, te->client_data);
      free(te);
      return ST_SUCCESS;
    }
    prev = te;
    te = te->next;
  }
  return ST_ERROR; /* NO event with the specified ID found */
}

/* Search the first timer to fire.
 * This operation is useful to know how many time the select can be
 * put in sleep without to delay any event.
 * If there are no timers NULL is returned.
 *
 * Note that's O(N) since time events are unsorted.
 * Possible optimizations (not needed by Redis so far, but...):
 * 1) Insert the event in order, so that the nearest is just the head.
 *    Much better but still insertion or deletion of timers is O(N).
 * 2) Use a skiplist to have this operation as O(1) and insertion as O(log(N)).
 */
TimeEvent* EventLoop::SearchNearestTimer() {
  TimeEvent *te = this->time_event_head_;
  TimeEvent *nearest = NULL;

  while (te) {
    if (!nearest || te->when_sec < nearest->when_sec
        || (te->when_sec == nearest->when_sec && te->when_ms < nearest->when_ms))
      nearest = te;
    te = te->next;
  }
  return nearest;
}

/* Process time events */
int EventLoop::ProcessTimeEvents() {
  int processed = 0;
  TimeEvent *te;
  long long maxId;
  time_t now = time(NULL);

  /* If the system clock is moved to the future, and then set back to the
   * right value, time events may be delayed in a random way. Often this
   * means that scheduled operations will not be performed soon enough.
   *
   * Here we try to detect system clock skews, and force all the time
   * events to be processed ASAP when this happens: the idea is that
   * processing events earlier is less dangerous than delaying them
   * indefinitely, and practice suggests it is. */
  if (now < this->last_time_) {
    te = this->time_event_head_;
    while (te) {
      te->when_sec = 0;
      te = te->next;
    }
  }
  this->last_time_ = now;

  te = this->time_event_head_;
  maxId = this->time_event_next_id_ - 1;
  while (te) {
    long now_sec, now_ms;
    long long id;

    if (te->id > maxId) {
      te = te->next;
      continue;
    }
    GetTime(&now_sec, &now_ms);
    if (now_sec > te->when_sec
        || (now_sec == te->when_sec && now_ms >= te->when_ms)) {
      int retval;

      id = te->id;
      retval = te->time_proc(this, id, te->client_data);
      processed++;
      /* After an event is processed our time event list may
       * no longer be the same, so we restart from head.
       * Still we make sure to don't process events registered
       * by event handlers itself in order to don't loop forever.
       * To do so we saved the max ID we want to handle.
       *
       * FUTURE OPTIMIZATIONS:
       * Note that this is NOT great algorithmically. Redis uses
       * a single time event so it's not a problem but the right
       * way to do this is to add the new elements on head, and
       * to flag deleted elements in a special way for later
       * deletion (putting references to the nodes to delete into
       * another linked list). */
      if (retval != AE_NOMORE) {
        AddMillisecondsToNow(retval, &te->when_sec, &te->when_ms);
      } else {
        this->DeleteTimeEvent(id);
      }
      te = this->time_event_head_;
    } else {
      te = te->next;
    }
  }
  return processed;
}

/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * Without special flags the function sleeps until some file event
 * fires, or when the next time event occurs (if any).
 *
 * If flags is 0, the function does nothing and returns.
 * if flags has AE_ALL_EVENTS set, all the kind of events are processed.
 * if flags has AE_FILE_EVENTS set, file events are processed.
 * if flags has AE_TIME_EVENTS set, time events are processed.
 * if flags has AE_DONT_WAIT set the function returns ASAP until all
 * the events that's possible to process without to wait are processed.
 *
 * The function returns the number of events processed. */
int EventLoop::ProcessEvents(int flags) {
  int processed = 0, numevents;

  /* Nothing to do? return ASAP */
  if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS))
    return 0;

  /* Note that we want call select() even if there are no
   * file events to process as long as we want to process time
   * events, in order to sleep until the next time event is ready
   * to fire. */
  if (this->maxfd_ != -1
      || ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
    int j;
    TimeEvent *shortest = NULL;
    struct timeval tv, *tvp;

    if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT))
      shortest = SearchNearestTimer();
    if (shortest) {
      long now_sec, now_ms;

      /* Calculate the time missing for the nearest
       * timer to fire. */
      GetTime(&now_sec, &now_ms);
      tvp = &tv;
      tvp->tv_sec = shortest->when_sec - now_sec;
      if (shortest->when_ms < now_ms) {
        tvp->tv_usec = ((shortest->when_ms + 1000) - now_ms) * 1000;
        tvp->tv_sec--;
      } else {
        tvp->tv_usec = (shortest->when_ms - now_ms) * 1000;
      }
      if (tvp->tv_sec < 0)
        tvp->tv_sec = 0;
      if (tvp->tv_usec < 0)
        tvp->tv_usec = 0;
    } else {
      /* If we have to check for events but need to return
       * ASAP because of AE_DONT_WAIT we need to set the timeout
       * to zero */
      if (flags & AE_DONT_WAIT) {
        tv.tv_sec = tv.tv_usec = 0;
        tvp = &tv;
      } else {
        /* Otherwise we can block */
        tvp = NULL; /* wait forever */
      }
    }

    numevents = EpollPoll(tvp);
    for (j = 0; j < numevents; j++) {
      FileEvent *fe = &this->events_[this->fired_[j].fd];
      int mask = this->fired_[j].mask;
      int fd = this->fired_[j].fd;
      int rfired = 0;

      /* note the fe->mask & mask & ... code: maybe an already processed
       * event removed an element that fired and we still didn't
       * processed, so we check if the event is still valid. */
      if (fe->mask & mask & AE_READABLE) {
        rfired = 1;
        fe->rfile_proc(this, fd, fe->client_data, mask);
      }
      if (fe->mask & mask & AE_WRITABLE) {
        if (!rfired || fe->wfile_proc != fe->rfile_proc)
          fe->wfile_proc(this, fd, fe->client_data, mask);
      }
      processed++;
    }
  }
  /* Check time events */
  if (flags & AE_TIME_EVENTS)
    processed += ProcessTimeEvents();

  return processed; /* return the number of processed file/time events */
}

void EventLoop::Start() {
  //start epoll
  this->stop_ = 0;
  while (!this->stop_) {
    ProcessEvents(AE_ALL_EVENTS);
  }
}
}  //end of namespace
