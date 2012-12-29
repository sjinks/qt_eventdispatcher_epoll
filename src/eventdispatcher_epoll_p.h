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

	enum datatype_t {
		dtTimer,
		dtSocketNotifier
	};

	struct data_t;

	struct SocketNotifierInfo {
		data_t* data;
		QSocketNotifier* sn;
		int events;
	};

	struct TimerInfo {
		data_t* data;
		QObject* object;
		struct timeval when;
		int timerId;
		int interval;
		int timerfd;
		Qt::TimerType type;
	};

	struct data_t {
		EventDispatcherEPollPrivate::datatype_t type;
		union {
			SocketNotifierInfo sni;
			TimerInfo ti;
		};
	};

	typedef QHash<QSocketNotifier*, SocketNotifierInfo*> SocketNotifierHash;
	typedef QHash<int, TimerInfo*> TimerHash;

private:
	Q_DISABLE_COPY(EventDispatcherEPollPrivate)
	Q_DECLARE_PUBLIC(EventDispatcherEPoll)
	EventDispatcherEPoll* const q_ptr;

	int m_epoll_fd;
	int m_event_fd;
	bool m_interrupt;
	QAtomicInt m_wakeups;
	SocketNotifierHash m_notifiers;
	TimerHash m_timers;

	static void calculateCoarseTimerTimeout(EventDispatcherEPollPrivate::TimerInfo* info, const struct timeval& now, struct timeval& when);
	static void calculateNextTimeout(EventDispatcherEPollPrivate::TimerInfo* info, const struct timeval& now, struct timeval& delta);

	void socket_notifier_callback(QSocketNotifier* n, int events);
	void timer_callback(EventDispatcherEPollPrivate::TimerInfo* info);
	void wake_up_handler(void);

	void disableSocketNotifiers(bool disable);
	void disableTimers(bool disable);
	void killTimers(void);
	void killSocketNotifiers(void);
};

#endif // EVENTDISPATCHER_EPOLL_P_H
