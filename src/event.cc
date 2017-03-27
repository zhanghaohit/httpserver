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
#include "event.h"
#include "log.h"

namespace httpserver{

static void aeGetTime(long *seconds, long *milliseconds)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec/1000;
}

static void aeAddMillisecondsToNow(long long milliseconds, long *sec, long *ms) {
    long cur_sec, cur_ms, when_sec, when_ms;

    aeGetTime(&cur_sec, &cur_ms);
    when_sec = cur_sec + milliseconds/1000;
    when_ms = cur_ms + milliseconds%1000;
    if (when_ms >= 1000) {
        when_sec ++;
        when_ms -= 1000;
    }
    *sec = when_sec;
    *ms = when_ms;
}

int aeEventLoop::aeApiAddEvent(int fd, int mask) {
    aeApiState *state = (aeApiState *)apidata;
    struct epoll_event ee;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = events[fd].mask == AE_NONE ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mask |= events[fd].mask; /* Merge old events */
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    if (mask & AE_EDEGE) ee.events |= EPOLLET;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (epoll_ctl(state->epfd,op,fd,&ee) == -1) return -1;
    return 0;
}

void aeEventLoop::aeApiDelEvent(int fd, int delmask) {
    aeApiState *state = (aeApiState *)apidata;
    struct epoll_event ee;
    int mask = events[fd].mask & (~delmask);

    ee.events = 0;
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (mask != AE_NONE) {
        epoll_ctl(state->epfd,EPOLL_CTL_MOD,fd,&ee);
    } else {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
         * EPOLL_CTL_DEL. */
        epoll_ctl(state->epfd,EPOLL_CTL_DEL,fd,&ee);
    }
}

int aeEventLoop::aeApiPoll(struct timeval *tvp) {
    aeApiState *state = (aeApiState *)apidata;
    int retval, numevents = 0;

    retval = epoll_wait(state->epfd,state->events,setsize,
            tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = state->events+j;

            LOG (LOG_WARNING, "event = %d", e->events);

            if (e->events & EPOLLIN) mask |= AE_READABLE;
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
            if (e->events & EPOLLERR) mask |= AE_WRITABLE;
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE;
            fired[j].fd = e->data.fd;
            fired[j].mask = mask;
        }
    }
    return numevents;
}

aeEventLoop::aeEventLoop(int setsize) {
    int i;

    this->events = (aeFileEvent *)malloc(sizeof(aeFileEvent)*setsize);
    this->fired = (aeFiredEvent *)malloc(sizeof(aeFiredEvent)*setsize);
    if (this->events == NULL || this->fired == NULL) return;
    this->setsize = setsize;
    this->lastTime = time(NULL);
    this->timeEventHead = NULL;
    this->timeEventNextId = 0;
    this->stop = 0;
    this->maxfd = -1;
    /* Events with mask == AE_NONE are not set. So let's initialize the
     * vector with it. */
    for (i = 0; i < setsize; i++)
        this->events[i].mask = AE_NONE;

    aeApiState *state = (aeApiState*)malloc(sizeof(aeApiState));
    if (!state) return;
    state->events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*this->setsize);
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
    this->apidata = state;
}

/* Resize the maximum set size of the event loop.
 * If the requested set size is smaller than the current set size, but
 * there is already a file descriptor in use that is >= the requested
 * set size minus one, AE_ERR is returned and the operation is not
 * performed at all.
 *
 * Otherwise AE_OK is returned and the operation is successful. */
int aeEventLoop::aeResizeSetSize(int setsize) {
    int i;

    if (setsize == this->setsize) return AE_SUCCESS;
    if (this->maxfd >= setsize) return AE_ERROR;

    aeApiState *state = static_cast<aeApiState*>(this->apidata);
    state->events = (struct epoll_event*)realloc(state->events, sizeof(struct epoll_event)*setsize);

    this->events = (aeFileEvent *)realloc(this->events,sizeof(aeFileEvent)*setsize);
    this->fired = (aeFiredEvent *)realloc(this->fired,sizeof(aeFiredEvent)*setsize);
    this->setsize = setsize;

    /* Make sure that if we created new slots, they are initialized with
     * an AE_NONE mask. */
    for (i = this->maxfd+1; i < setsize; i++)
        this->events[i].mask = AE_NONE;
    return AE_SUCCESS;
}

aeEventLoop::~aeEventLoop() {
    aeApiState *state = (aeApiState *)apidata;
    close(state->epfd);
    free(state->events);
    free(state);

    free(this->events);
    free(this->fired);
}

void aeEventLoop::aeStop() {
    this->stop = 1;
}

int aeEventLoop::aeCreateFileEvent(int fd, int mask,
        aeFileProc *proc, void *clientData)
{
	if (fd >= this->setsize) {
		aeResizeSetSize(this->setsize*2);
	}

    if (fd >= this->setsize) {
        errno = ERANGE;
        return AE_ERROR;
    }
    aeFileEvent *fe = &this->events[fd];

    if (aeApiAddEvent(fd, mask) == -1)
        return AE_ERROR;
    fe->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    fe->clientData = clientData;
    if (fd > this->maxfd)
        this->maxfd = fd;
    return AE_SUCCESS;
}

void aeEventLoop::aeDeleteFileEvent(int fd, int mask)
{
    if (fd >= this->setsize) return;
    aeFileEvent *fe = &this->events[fd];
    if (fe->mask == AE_NONE) return;

    aeApiDelEvent(fd, mask);
    fe->mask = fe->mask & (~mask);
    if (fd == this->maxfd && fe->mask == AE_NONE) {
        /* Update the max fd */
        int j;

        for (j = this->maxfd-1; j >= 0; j--)
            if (this->events[j].mask != AE_NONE) break;
        this->maxfd = j;
    }
}

int aeEventLoop::aeGetFileEvents(int fd) {
    if (fd >= this->setsize) return 0;
    aeFileEvent *fe = &this->events[fd];

    return fe->mask;
}

long long aeEventLoop::aeCreateTimeEvent(long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc)
{
    long long id = this->timeEventNextId++;
    aeTimeEvent *te;

    te = (aeTimeEvent *)malloc(sizeof(*te));
    if (te == NULL) return AE_ERROR;
    te->id = id;
    aeAddMillisecondsToNow(milliseconds,&te->when_sec,&te->when_ms);
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    te->clientData = clientData;
    te->next = this->timeEventHead;
    this->timeEventHead = te;
    return id;
}

int aeEventLoop::aeDeleteTimeEvent(long long id)
{
    aeTimeEvent *te, *prev = NULL;

    te = this->timeEventHead;
    while(te) {
        if (te->id == id) {
            if (prev == NULL)
                this->timeEventHead = te->next;
            else
                prev->next = te->next;
            if (te->finalizerProc)
                te->finalizerProc(this, te->clientData);
            free(te);
            return AE_SUCCESS;
        }
        prev = te;
        te = te->next;
    }
    return AE_ERROR; /* NO event with the specified ID found */
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
aeTimeEvent* aeEventLoop::aeSearchNearestTimer() {
    aeTimeEvent *te = this->timeEventHead;
    aeTimeEvent *nearest = NULL;

    while(te) {
        if (!nearest || te->when_sec < nearest->when_sec ||
                (te->when_sec == nearest->when_sec &&
                 te->when_ms < nearest->when_ms))
            nearest = te;
        te = te->next;
    }
    return nearest;
}

/* Process time events */
int aeEventLoop::processTimeEvents() {
    int processed = 0;
    aeTimeEvent *te;
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
    if (now < this->lastTime) {
        te = this->timeEventHead;
        while(te) {
            te->when_sec = 0;
            te = te->next;
        }
    }
    this->lastTime = now;

    te = this->timeEventHead;
    maxId = this->timeEventNextId-1;
    while(te) {
        long now_sec, now_ms;
        long long id;

        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        aeGetTime(&now_sec, &now_ms);
        if (now_sec > te->when_sec ||
            (now_sec == te->when_sec && now_ms >= te->when_ms))
        {
            int retval;

            id = te->id;
            retval = te->timeProc(this, id, te->clientData);
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
                aeAddMillisecondsToNow(retval,&te->when_sec,&te->when_ms);
            } else {
            	this->aeDeleteTimeEvent(id);
            }
            te = this->timeEventHead;
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
int aeEventLoop::aeProcessEvents(int flags)
{
    int processed = 0, numevents;

    /* Nothing to do? return ASAP */
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) return 0;

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */
    if (this->maxfd != -1 ||
        ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
        int j;
        aeTimeEvent *shortest = NULL;
        struct timeval tv, *tvp;

        if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT))
            shortest = aeSearchNearestTimer();
        if (shortest) {
            long now_sec, now_ms;

            /* Calculate the time missing for the nearest
             * timer to fire. */
            aeGetTime(&now_sec, &now_ms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec - now_sec;
            if (shortest->when_ms < now_ms) {
                tvp->tv_usec = ((shortest->when_ms+1000) - now_ms)*1000;
                tvp->tv_sec --;
            } else {
                tvp->tv_usec = (shortest->when_ms - now_ms)*1000;
            }
            if (tvp->tv_sec < 0) tvp->tv_sec = 0;
            if (tvp->tv_usec < 0) tvp->tv_usec = 0;
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

        numevents = aeApiPoll(tvp);
        for (j = 0; j < numevents; j++) {
            aeFileEvent *fe = &this->events[this->fired[j].fd];
            int mask = this->fired[j].mask;
            int fd = this->fired[j].fd;
            int rfired = 0;

	    /* note the fe->mask & mask & ... code: maybe an already processed
             * event removed an element that fired and we still didn't
             * processed, so we check if the event is still valid. */
            if (fe->mask & mask & AE_READABLE) {
                rfired = 1;
                fe->rfileProc(this,fd,fe->clientData,mask);
            }
            if (fe->mask & mask & AE_WRITABLE) {
                if (!rfired || fe->wfileProc != fe->rfileProc)
                    fe->wfileProc(this,fd,fe->clientData,mask);
            }
            processed++;
        }
    }
    /* Check time events */
    if (flags & AE_TIME_EVENTS)
        processed += processTimeEvents();

    return processed; /* return the number of processed file/time events */
}

void aeEventLoop::startEventLoop() {
	//start epoll
    this->stop = 0;
    while (!this->stop) {
        aeProcessEvents(AE_ALL_EVENTS);
    }
}
} //end of namespace
