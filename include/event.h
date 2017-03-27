#ifndef INCLUDE_AE_H_
#define INCLUDE_AE_H_

#include <time.h>

namespace httpserver {

#define AE_SUCCESS 0
#define AE_ERROR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2
#define AE_EDEGE 4

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1

struct aeEventLoop;

/* Types and data structures */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);

/* File event structure */
struct aeFileEvent {
    int mask; /* one of AE_(READABLE|WRITABLE) */
    aeFileProc *rfileProc;
    aeFileProc *wfileProc;
    void *clientData;
};

/* Time event structure */
struct aeTimeEvent {
    long long id; /* time event identifier. */
    long when_sec; /* seconds */
    long when_ms; /* milliseconds */
    aeTimeProc *timeProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeTimeEvent *next;
};

/* A fired event */
struct aeFiredEvent {
    int fd;
    int mask;
};

struct aeApiState {
    int epfd;
    struct epoll_event *events;
};

class aeEventLoop {
public:
	aeEventLoop(int setsize);
	~aeEventLoop();

	void startEventLoop();
	void aeStop();

	int aeCreateFileEvent(int fd, int mask,
	        aeFileProc *proc, void *clientData);
	void aeDeleteFileEvent(int fd, int mask);
	int aeGetFileEvents(int fd);
	long long aeCreateTimeEvent(long long milliseconds,
	        aeTimeProc *proc, void *clientData,
	        aeEventFinalizerProc *finalizerProc);
	int aeDeleteTimeEvent(long long id);

private:
	int aeResizeSetSize(int setsize);

	int aeProcessEvents(int flags);
	int processTimeEvents();
	aeTimeEvent* aeSearchNearestTimer();
	int aeApiAddEvent(int fd, int mask);
	void aeApiDelEvent(int fd, int delmask);
	int aeApiPoll(struct timeval *tvp);

    int maxfd;   /* highest file descriptor currently registered */
    int setsize; /* max number of file descriptors tracked */
    long long timeEventNextId;
    time_t lastTime;     /* Used to detect system clock skew */
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    aeTimeEvent *timeEventHead;
    int stop;
    void *apidata; /* This is used for polling API specific data */
};
}

#endif
