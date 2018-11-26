#ifndef EVENTDISPATCHER_EPOLL_QPA_H
#define EVENTDISPATCHER_EPOLL_QPA_H

#include "eventdispatcher_epoll.h"

#if QT_VERSION < 0x050000
#	error This code requires at least Qt 5
#endif

class EventDispatcherEPollQPA : public EventDispatcherEPoll {
	Q_OBJECT
public:
	explicit EventDispatcherEPollQPA(QObject* parent = 0);
	virtual ~EventDispatcherEPollQPA(void) Q_DECL_OVERRIDE;

	bool processEvents(QEventLoop::ProcessEventsFlags flags) Q_DECL_OVERRIDE;
	bool hasPendingEvents(void) Q_DECL_OVERRIDE;
	void flush(void) Q_DECL_OVERRIDE;

private:
	Q_DISABLE_COPY(EventDispatcherEPollQPA)
};

#endif // EVENTDISPATCHER_EPOLL_QPA_H
