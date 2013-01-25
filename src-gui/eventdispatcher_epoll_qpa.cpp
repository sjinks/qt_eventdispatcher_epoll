#include <qplatformdefs.h>
#include <qpa/qwindowsysteminterface.h>
#include <QtGui/QGuiApplication>
#include "eventdispatcher_epoll_qpa.h"

EventDispatcherEPollQPA::EventDispatcherEPollQPA(QObject* parent)
	: EventDispatcherEPoll(parent)
{
}

EventDispatcherEPollQPA::~EventDispatcherEPollQPA(void)
{
}

bool EventDispatcherEPollQPA::processEvents(QEventLoop::ProcessEventsFlags flags)
{
	bool sent_events = QWindowSystemInterface::sendWindowSystemEvents(flags);

	if (EventDispatcherEPoll::processEvents(flags)) {
		return true;
	}

	return sent_events;
}

bool EventDispatcherEPollQPA::hasPendingEvents(void)
{
	return EventDispatcherEPoll::hasPendingEvents() || QWindowSystemInterface::windowSystemEventsQueued();
}

void EventDispatcherEPollQPA::flush(void)
{
	if (qApp) {
		qApp->sendPostedEvents();
	}
}
