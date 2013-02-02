#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include "eventdispatcher_epoll.h"

class EventDispatcher : public EventDispatcherEPoll {
	Q_OBJECT
public:
	explicit EventDispatcher(QObject* parent = 0) : EventDispatcherEPoll(parent) {}
};

#endif // EVENTDISPATCHER_H
