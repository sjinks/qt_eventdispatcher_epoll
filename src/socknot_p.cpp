#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QSocketNotifier>
#include <sys/epoll.h>
#include "eventdispatcher_epoll_p.h"

static inline int sn2et(QSocketNotifier::Type t)
{
	switch (t) {
		case QSocketNotifier::Read:      return EPOLLIN;
		case QSocketNotifier::Write:     return EPOLLOUT;
		case QSocketNotifier::Exception: return EPOLLPRI;
		default:
			Q_UNREACHABLE();
	}
}

void EventDispatcherEPollPrivate::registerSocketNotifier(QSocketNotifier* notifier)
{
	int events = sn2et(notifier->type());

	HandleData* data = new EventDispatcherEPollPrivate::HandleData();
	data->type       = EventDispatcherEPollPrivate::htSocketNotifier;
	data->sni.sn     = notifier;

	int fd = static_cast<int>(notifier->socket());

	epoll_event e;
	e.events  = events;
	e.data.fd = fd;

	epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, fd, &e);
	this->m_notifiers.insert(notifier, data);
	this->m_handles.insert(fd, data);
}

void EventDispatcherEPollPrivate::unregisterSocketNotifier(QSocketNotifier* notifier)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.find(notifier);
	if (it != this->m_notifiers.end()) {
		int fd = static_cast<int>(notifier->socket());
		delete it.value();
		this->m_notifiers.erase(it); // Hash is not rehashed
		this->m_handles.remove(fd);
		epoll_ctl(this->m_epoll_fd, EPOLL_CTL_DEL, fd, 0);
	}
}

void EventDispatcherEPollPrivate::socket_notifier_callback(QSocketNotifier* n, int events)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.find(n);
	if (it != this->m_notifiers.end()) {
		HandleData* data = it.value();
		if (data->type == EventDispatcherEPollPrivate::htSocketNotifier) {
			QSocketNotifier::Type type = data->sni.sn->type();

			if (
				   (QSocketNotifier::Read      == type && (events & EPOLLIN))
				|| (QSocketNotifier::Write     == type && (events & EPOLLOUT))
				|| (QSocketNotifier::Exception == type && (events && EPOLLPRI))
			) {
				QEvent e(QEvent::SockAct);
				QCoreApplication::sendEvent(data->sni.sn, &e);
			}
		}
		else {
			Q_UNREACHABLE();
		}
	}
}

void EventDispatcherEPollPrivate::disableSocketNotifiers(bool disable)
{
	epoll_event e;

	SocketNotifierHash::ConstIterator it = this->m_notifiers.constBegin();
	while (it != this->m_notifiers.constEnd()) {
		QSocketNotifier* notifier = it.key();
		int fd                    = static_cast<int>(notifier->socket());

		e.events  = disable ? 0 : sn2et(notifier->type());
		e.data.fd = fd;
		epoll_ctl(this->m_event_fd, EPOLL_CTL_MOD, fd, &e);
		++it;
	}
}

void EventDispatcherEPollPrivate::killSocketNotifiers(void)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.begin();
	while (it != this->m_notifiers.end()) {
		HandleData* data = it.value();
		if (data->type == EventDispatcherEPollPrivate::htSocketNotifier) {
			int fd = static_cast<int>(it.key()->socket());

			delete data;
			it = this->m_notifiers.erase(it);
			this->m_handles.remove(fd);
		}
		else {
			Q_UNREACHABLE();
		}
	}
}
