QT       -= gui
TARGET    = eventdispatcher_epoll
TEMPLATE  = lib
DESTDIR   = ../lib
CONFIG   += staticlib create_prl create_pc
HEADERS  += eventdispatcher_epoll.h eventdispatcher_epoll_p.h qt4compat.h
SOURCES  += eventdispatcher_epoll.cpp eventdispatcher_epoll_p.cpp timers_p.cpp socknot_p.cpp

headers.files = eventdispatcher_epoll.h
headers.path  = /usr/include
target.path   = /usr/lib

QMAKE_PKGCONFIG_NAME        = eventdispatcher_epoll
QMAKE_PKGCONFIG_DESCRIPTION = "epoll()-based event dispatcher for Qt"
QMAKE_PKGCONFIG_LIBDIR      = $$target.path
QMAKE_PKGCONFIG_INCDIR      = $$headers.path
QMAKE_PKGCONFIG_DESTDIR     = pkgconfig

INSTALLS += target headers
