TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS = src tests

src.file   = src/eventdispatcher_epoll.pro
tests.file = tests/qt_eventdispatcher_tests/build.pro
