#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QSocketNotifier>
#include <sys/epoll.h>
#include <errno.h>
#include "eventdispatcher_epoll_p.h"
#include "qt4compat.h"

void EventDispatcherEPollPrivate::registerSocketNotifier(QSocketNotifier* notifier)
{
	int events;
	QSocketNotifier** n;
	int fd = static_cast<int>(notifier->socket());

	epoll_event e;
	e.data.fd = fd;

	HandleData* data;
	HandleHash::Iterator it = this->m_handles.find(fd);

	if (it == this->m_handles.end()) {
		data        = new EventDispatcherEPollPrivate::HandleData();
		data->type  = EventDispatcherEPollPrivate::htSocketNotifier;
		data->sni.r = 0;
		data->sni.w = 0;
		data->sni.x = 0;

		switch (notifier->type()) {
			case QSocketNotifier::Read:      events = EPOLLIN;  n = &data->sni.r; break;
			case QSocketNotifier::Write:     events = EPOLLOUT; n = &data->sni.w; break;
			case QSocketNotifier::Exception: events = EPOLLPRI; n = &data->sni.x; break;
			default:
				Q_UNREACHABLE();
		}

		data->sni.events = events;
		e.events         = events;
		*n               = notifier;

		int res = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, fd, &e);
		if (Q_UNLIKELY(res != 0)) {
			qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
		}

		this->m_handles.insert(fd, data);
	}
	else {
		data = it.value();

		Q_ASSERT(data->type == EventDispatcherEPollPrivate::htSocketNotifier);
		if (data->type == EventDispatcherEPollPrivate::htSocketNotifier) {
			switch (notifier->type()) {
				case QSocketNotifier::Read:      events = EPOLLIN;  n = &data->sni.r; break;
				case QSocketNotifier::Write:     events = EPOLLOUT; n = &data->sni.w; break;
				case QSocketNotifier::Exception: events = EPOLLPRI; n = &data->sni.x; break;
				default:
					Q_UNREACHABLE();
			}

			if (Q_UNLIKELY(*n != 0)) {
				qWarning("%s: cannot add two socket notifiers of the same type for the same descriptor", Q_FUNC_INFO);
				return;
			}

			data->sni.events |= events;
			e.events          = data->sni.events;
			*n                = notifier;

			int res = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
			if (Q_UNLIKELY(res != 0)) {
				qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
			}
		}
		else {
			Q_UNREACHABLE();
		}
	}

	this->m_notifiers.insert(notifier, data);
}

void EventDispatcherEPollPrivate::unregisterSocketNotifier(QSocketNotifier* notifier)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.find(notifier);
	if (it != this->m_notifiers.end()) {
		HandleData* info = it.value();
		int fd           = static_cast<int>(notifier->socket());

		this->m_notifiers.erase(it); // Hash is not rehashed

		HandleHash::Iterator hi = this->m_handles.find(fd);
		Q_ASSERT(hi != this->m_handles.end());

		struct epoll_event e;
		e.data.fd = fd;

		if (info->sni.r == notifier) {
			info->sni.events &= ~EPOLLIN;
			info->sni.r       = 0;
		}
		else if (info->sni.w == notifier) {
			info->sni.events &= ~EPOLLOUT;
			info->sni.w       = 0;
		}
		else if (info->sni.x == notifier) {
			info->sni.events &= ~EPOLLPRI;
			info->sni.x       = 0;
		}
		else {
			qCritical("%s: cannot find socket notifier %p", Q_FUNC_INFO, notifier);
		}

		e.events = info->sni.events;

		int res;

		if (info->sni.r || info->sni.w || info->sni.x) {
			res = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
		}
		else {
			res = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_DEL, fd, &e);
			if (res != 0 && EBADF == errno) {
				res = 0;
			}

			this->m_handles.erase(hi);
			delete info;
		}

		if (Q_UNLIKELY(res != 0)) {
			qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
		}
	}
}

void EventDispatcherEPollPrivate::socket_notifier_callback(SocketNotifierInfo *n, int events)
{
	QEvent e(QEvent::SockAct);

	if (n->r && (events & EPOLLIN)) {
		QCoreApplication::sendEvent(n->r, &e);
	}

	if (n->w && (events & EPOLLOUT)) {
		QCoreApplication::sendEvent(n->w, &e);
	}

	if (n->x && (events & EPOLLPRI)) {
		QCoreApplication::sendEvent(n->x, &e);
	}
}

void EventDispatcherEPollPrivate::disableSocketNotifiers(bool disable)
{
	epoll_event e;

	SocketNotifierHash::ConstIterator it = this->m_notifiers.constBegin();
	while (it != this->m_notifiers.constEnd()) {
		QSocketNotifier* notifier = it.key();
		HandleData* info          = it.value();
		int fd                    = static_cast<int>(notifier->socket());

		e.events  = disable ? 0 : info->sni.events;
		e.data.fd = fd;
		int res = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_MOD, fd, &e);
		if (Q_UNLIKELY(res != 0)) {
			qErrnoWarning("%s: epoll_ctl() failed %d %d", Q_FUNC_INFO, disable, e.events);
		}

		++it;
	}
}

void EventDispatcherEPollPrivate::killSocketNotifiers(void)
{
	SocketNotifierHash::Iterator it = this->m_notifiers.begin();
	while (it != this->m_notifiers.end()) {
		HandleData* data = it.value();

		Q_ASSERT(data->type == EventDispatcherEPollPrivate::htSocketNotifier);
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
