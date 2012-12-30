#ifndef QT4COMPAT_H
#define QT4COMPAT_H

#include <QtCore/QtGlobal>

#ifndef Q_LIKELY
#	define Q_LIKELY(s) s
#endif

#ifndef Q_UNLIKELY
#	define Q_UNLIKELY(s) s
#endif

#ifndef Q_UNREACHABLE
#	define Q_UNREACHABLE() Q_ASSERT(false)
#endif

#ifndef Q_ASSUME
#	define Q_ASSUME(s) if (s) {} else { Q_UNREACHABLE(); }
#endif

#endif // QT4COMPAT_H
