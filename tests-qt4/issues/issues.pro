CONFIG  += testcase
TARGET   = tst_issues
QT       = core network testlib
SOURCES += tst_issues.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../

include(../common.pri)
