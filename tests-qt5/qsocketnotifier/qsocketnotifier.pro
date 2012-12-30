CONFIG  += testcase
TARGET   = tst_qsocketnotifier
QT       = core-private network-private testlib network
HEADERS  = qabstractsocketengine_p.h qnativesocketengine_p.h
SOURCES  = tst_qsocketnotifier.cpp qabstractsocketengine.cpp qnativesocketengine.cpp qnativesocketengine_unix.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../

QNETWORK_SRC = $$QT_SOURCE_TREE/src/network
INCLUDEPATH += $$QNETWORK_SRC

include(../common.pri)
