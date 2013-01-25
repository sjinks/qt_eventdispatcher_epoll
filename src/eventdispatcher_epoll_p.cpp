#include <QtCore/QCoreApplication>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdlib.h>
#include <errno.h>
#include "eventdispatcher_epoll.h"
#include "eventdispatcher_epoll_p.h"
#include "qt4compat.h"

EventDispatcherEPollPrivate::EventDispatcherEPollPrivate(EventDispatcherEPoll* const q)
	: q_ptr(q),
	  m_epoll_fd(-1), m_event_fd(-1),
	  m_interrupt(false), m_wakeups(),
	  m_handles(), m_notifiers(), m_timers()
{
	this->m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (-1 == this->m_epoll_fd) {
		qErrnoWarning("epoll_create1() failed");
		abort();
	}

	this->m_event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (-1 == this->m_event_fd) {
		qErrnoWarning("eventfd() failed");
		abort();
	}

	struct epoll_event e;
	e.events  = EPOLLIN;
	e.data.fd = this->m_event_fd;
	if (-1 == epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, this->m_event_fd, &e)) {
		qErrnoWarning("%s: epoll_ctl() failed", Q_FUNC_INFO);
	}
}

EventDispatcherEPollPrivate::~EventDispatcherEPollPrivate(void)
{
	close(this->m_event_fd);
	close(this->m_epoll_fd);

	HandleHash::Iterator it = this->m_handles.begin();
	while (it != this->m_handles.end()) {
		delete it.value();
		++it;
	}
}

bool EventDispatcherEPollPrivate::processEvents(QEventLoop::ProcessEventsFlags flags)
{
	Q_Q(EventDispatcherEPoll);

	bool exclude_notifiers = (flags & QEventLoop::ExcludeSocketNotifiers);
	bool exclude_timers    = (flags & QEventLoop::X11ExcludeTimers);

	if (exclude_notifiers) {
		this->disableSocketNotifiers(true);
	}

	if (exclude_timers) {
		this->disableTimers(true);
	}

	this->m_interrupt = false;
	Q_EMIT q->awake();
	QCoreApplication::sendPostedEvents();

	int n_events  = 0;
	bool can_wait = !this->m_interrupt && (flags & QEventLoop::WaitForMoreEvents);
	int timeout   = 0;

	if (can_wait) {
		Q_EMIT q->aboutToBlock();
		timeout = -1;
	}

	if (!this->m_interrupt) {
		struct epoll_event events[1024];
		do {
			n_events = epoll_wait(this->m_epoll_fd, events, 1024, timeout);
		} while (-1 == n_events && errno == EINTR);

		for (int i=0; i<n_events; ++i) {
			struct epoll_event& e = events[i];
			int fd                = e.data.fd;
			if (fd == this->m_event_fd) {
				if (Q_LIKELY(e.events & EPOLLIN)) {
					this->wake_up_handler();
				}
			}
			else {
				HandleHash::ConstIterator it = this->m_handles.find(fd);
				if (Q_LIKELY(it != this->m_handles.constEnd())) {
					HandleData* data = it.value();
					switch (data->type) {
						case EventDispatcherEPollPrivate::htSocketNotifier:
							this->socket_notifier_callback(&data->sni, e.events);
							break;

						case EventDispatcherEPollPrivate::htTimer:
							this->timer_callback(&data->ti);
							break;

						default:
							Q_UNREACHABLE();
					}
				}
			}
		}
	}

	if (exclude_notifiers) {
		this->disableSocketNotifiers(false);
	}

	if (exclude_timers) {
		this->disableTimers(false);
	}

	return n_events > 0;
}

void EventDispatcherEPollPrivate::wake_up_handler(void)
{
	eventfd_t value;
	int res;
	do {
		res = eventfd_read(this->m_event_fd, &value);
	} while (-1 == res && EINTR == errno);

	if (Q_UNLIKELY(-1 == res)) {
		qErrnoWarning("%s: eventfd_read() failed", Q_FUNC_INFO);
	}

	if (Q_UNLIKELY(!this->m_wakeups.testAndSetRelease(1, 0))) {
		qCritical("%s: internal error, testAndSetRelease(1, 0) failed!", Q_FUNC_INFO);
	}
}

void EventDispatcherEPollPrivate::wakeup(void)
{
	if (this->m_wakeups.testAndSetAcquire(0, 1)) {
		const eventfd_t value = 1;
		int res;

		do {
			res = eventfd_write(this->m_event_fd, value);
		} while (-1 == res && EINTR == errno);

		if (Q_UNLIKELY(-1 == res)) {
			qErrnoWarning("%s: eventfd_write() failed", Q_FUNC_INFO);
		}
	}
}
