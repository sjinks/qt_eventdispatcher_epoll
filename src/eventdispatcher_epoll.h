#ifndef EVENTDISPATCHER_EPOLL_H
#define EVENTDISPATCHER_EPOLL_H

#include <QtCore/QAbstractEventDispatcher>

class EventDispatcherEPollPrivate;
class EventDispatcherLibEventConfig;

class EventDispatcherEPoll : public QAbstractEventDispatcher {
	Q_OBJECT
public:
	explicit EventDispatcherEPoll(QObject* parent = 0);
	virtual ~EventDispatcherEPoll(void);

	virtual bool processEvents(QEventLoop::ProcessEventsFlags flags);
	virtual bool hasPendingEvents(void);

	virtual void registerSocketNotifier(QSocketNotifier* notifier);
	virtual void unregisterSocketNotifier(QSocketNotifier* notifier);

	virtual void registerTimer(
		int timerId,
		int interval,
#if QT_VERSION >= 0x050000
		Qt::TimerType timerType,
#endif
		QObject* object
	);

	virtual bool unregisterTimer(int timerId);
	virtual bool unregisterTimers(QObject* object);
	virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject* object) const;
#if QT_VERSION >= 0x050000
	virtual int remainingTime(int timerId);
#endif

	virtual void wakeUp(void);
	virtual void interrupt(void);
	virtual void flush(void);

private:
	Q_DISABLE_COPY(EventDispatcherEPoll)
	Q_DECLARE_PRIVATE(EventDispatcherEPoll)
	QScopedPointer<EventDispatcherEPollPrivate> d_ptr;
};

#endif // EVENTDISPATCHER_EPOLL_H
