QT      += gui-private
CONFIG  += staticlib create_prl link_prl
TEMPLATE = lib
TARGET   = eventdispatcher_epoll_qpa
SOURCES  = eventdispatcher_epoll_qpa.cpp
HEADERS  = eventdispatcher_epoll_qpa.h
DESTDIR  = ../lib

LIBS           += -L$$PWD/../lib -leventdispatcher_epoll
INCLUDEPATH    += $$PWD/../src
DEPENDPATH     += $$PWD/../src
PRE_TARGETDEPS += $$DESTDIR/../lib/libeventdispatcher_epoll.a
