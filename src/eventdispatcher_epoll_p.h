#ifndef EVENTDISPATCHER_EPOLL_P_H
#define EVENTDISPATCHER_EPOLL_P_H

#include <qplatformdefs.h>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QAtomicInt>
#include <QtCore/QHash>

#if QT_VERSION < 0x050000
namespace Qt { // Sorry
	enum TimerType {
		PreciseTimer,
		CoarseTimer,
		VeryCoarseTimer
	};
}
#endif

enum HandleType {
	htTimer,
	htSocketNotifier
};

struct Q_DECL_HIDDEN SocketNotifierInfo {
	QSocketNotifier* r;
	QSocketNotifier* w;
	QSocketNotifier* x;
	int events;
};

struct Q_DECL_HIDDEN TimerInfo {
	QObject* object;
	struct timeval when;
	int timerId;
	int interval;
	int fd;
	Qt::TimerType type;
};

struct HandleData {
	HandleType type;
	union {
		SocketNotifierInfo sni;
		TimerInfo ti;
	};
};

Q_DECLARE_TYPEINFO(SocketNotifierInfo, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(TimerInfo, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(HandleData, Q_PRIMITIVE_TYPE);

class EventDispatcherEPoll;

class Q_DECL_HIDDEN EventDispatcherEPollPrivate {
public:
	EventDispatcherEPollPrivate(EventDispatcherEPoll* const q);
	~EventDispatcherEPollPrivate(void);
	bool processEvents(QEventLoop::ProcessEventsFlags flags);
	void registerSocketNotifier(QSocketNotifier* notifier);
	void unregisterSocketNotifier(QSocketNotifier* notifier);
	void registerTimer(int timerId, int interval, Qt::TimerType type, QObject* object);
	bool unregisterTimer(int timerId);
	bool unregisterTimers(QObject* object);
	QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const;
	int remainingTime(int timerId) const;
	void wakeup(void);

	typedef QHash<int, HandleData*> HandleHash;
	typedef QHash<int, HandleData*> TimerHash;
	typedef QHash<QSocketNotifier*, HandleData*> SocketNotifierHash;

private:
	Q_DISABLE_COPY(EventDispatcherEPollPrivate)
	Q_DECLARE_PUBLIC(EventDispatcherEPoll)
	EventDispatcherEPoll* const q_ptr;

	int m_epoll_fd;
	int m_event_fd;
	bool m_interrupt;
	QAtomicInt m_wakeups;
	HandleHash m_handles;
	SocketNotifierHash m_notifiers;
	TimerHash m_timers;

	static void calculateCoarseTimerTimeout(TimerInfo* info, const struct timeval& now, struct timeval& when);
	static void calculateNextTimeout(TimerInfo* info, const struct timeval& now, struct timeval& delta);

	void socket_notifier_callback(SocketNotifierInfo* n, int events);
	void timer_callback(TimerInfo* info);
	void wake_up_handler(void);

	void disableSocketNotifiers(bool disable);
	void disableTimers(bool disable);
};

#endif // EVENTDISPATCHER_EPOLL_P_H
