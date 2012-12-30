#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QSocketNotifier>
#include <sys/epoll.h>
#include "eventdispatcher_epoll_p.h"

void EventDispatcherEPollPrivate::registerSocketNotifier(QSocketNotifier* notifier)
{
	int events;
	switch (notifier->type()) {
		case QSocketNotifier::Read:      events = EPOLLIN; break;
		case QSocketNotifier::Write:     events = EPOLLOUT; break;
		case QSocketNotifier::Exception: events = EPOLLPRI; break;
		default:
			Q_ASSERT(false);
			return;
	}

	EventDispatcherEPollPrivate::data_t* data = new EventDispatcherEPollPrivate::data_t();
	data->type       = EventDispatcherEPollPrivate::dtSocketNotifier;
	data->sni.data   = data;
	data->sni.sn     = notifier;
	data->sni.events = events;

	epoll_event e;
	e.events   = events;
	e.data.ptr = data;

	epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, static_cast<int>(notifier->socket()), &e);
	this->m_notifiers.insert(notifier, &data->sni);
}

void EventDispatcherEPollPrivate::unregisterSocketNotifier(QSocketNotifier* notifier)
{
	EventDispatcherEPollPrivate::SocketNotifierHash::Iterator it = this->m_notifiers.find(notifier);
	if (it != this->m_notifiers.end()) {
		int socket = static_cast<int>(notifier->socket());
		delete it.value()->data;
		this->m_notifiers.erase(it); // Hash is not rehashed
		epoll_ctl(this->m_epoll_fd, EPOLL_CTL_DEL, socket, 0);
	}
}

void EventDispatcherEPollPrivate::socket_notifier_callback(QSocketNotifier* n, int events)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.find(n);
	if (it != this->m_notifiers.end()) {
		EventDispatcherEPollPrivate::SocketNotifierInfo* data = it.value();
		QSocketNotifier::Type type = data->sn->type();

		if (
			   (QSocketNotifier::Read      == type && (events & EPOLLIN))
			|| (QSocketNotifier::Write     == type && (events & EPOLLOUT))
			|| (QSocketNotifier::Exception == type && (events && EPOLLPRI))
		) {
			QEvent e(QEvent::SockAct);
			QCoreApplication::sendEvent(data->sn, &e);
		}
	}
}

void EventDispatcherEPollPrivate::disableSocketNotifiers(bool disable)
{
	epoll_event e;

	EventDispatcherEPollPrivate::SocketNotifierHash::ConstIterator it = this->m_notifiers.constBegin();
	while (it != this->m_notifiers.constEnd()) {
		QSocketNotifier* notifier = it.key();
		SocketNotifierInfo* info  = it.value();
		int socket                = static_cast<int>(notifier->socket());

		e.events   = disable ? 0 : info->events;
		e.data.ptr = info->data;
		epoll_ctl(this->m_event_fd, EPOLL_CTL_MOD, socket, &e);
		++it;
	}
}

void EventDispatcherEPollPrivate::killSocketNotifiers(void)
{
	EventDispatcherEPollPrivate::SocketNotifierHash::Iterator it = this->m_notifiers.begin();
	while (it != this->m_notifiers.end()) {
		SocketNotifierInfo* info = it.value();
		delete info->data;
		it = this->m_notifiers.erase(it);
	}
}
