#ifndef QT4COMPAT_H
#define QT4COMPAT_H

#include <QtCore/QtGlobal>

#if !defined(Q_UNREACHABLE) && !defined(Q_ASSUME)
#	if defined(Q_CC_INTEL) || defined(Q_CC_MSVC)
#		define Q_UNREACHABLE() __assume(0)
#		define Q_ASSUME(s) __assume(s)
#	elif defined(Q_CC_CLANG)
#		define Q_UNREACHABLE() __builtin_unreachable()
#	elif defined(Q_CC_GNU) && defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
#		define Q_UNREACHABLE() __builtin_unreachable()
#	endif
#endif

#if !defined(Q_LIKELY) && !defined(Q_UNLIKELY)
#	ifdef __GNUC__
#		define Q_LIKELY(expr)    __builtin_expect(!!(expr), true)
#		define Q_UNLIKELY(expr)  __builtin_expect(!!(expr), false)
#	endif
#endif

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
