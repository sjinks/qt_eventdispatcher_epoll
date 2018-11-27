#ifndef EVENTDISPATCHERQPA_H
#define EVENTDISPATCHERQPA_H

#include "eventdispatcher_epoll_qpa.h"

class EventDispatcherQPA : public EventDispatcherEPollQPA {
	Q_OBJECT
public:
	explicit EventDispatcherQPA(QObject* parent = 0) : EventDispatcherEPollQPA(parent) {}
};

#endif // EVENTDISPATCHERQPA_H
