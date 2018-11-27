INCLUDEPATH += $$PWD $$PWD/../src
DEPENDPATH  += $$PWD $$PWD/../src

HEADERS += $$PWD/eventdispatcher.h

CONFIG  *= link_prl
LIBS    += -L$$OUT_PWD/$$DESTDIR/../lib

!CONFIG(console) {
    HEADERS     += $$PWD/eventdispatcherqpa.h
    LIBS        += -leventdispatcher_epoll_qpa
    INCLUDEPATH += $$PWD/../src-gui
    DEPENDPATH  += $$PWD/../src-gui

    win32: CONFIG += windows
}
else {
    LIBS    += -leventdispatcher_epoll
}

unix|*-g++* {
    equals(QMAKE_PREFIX_STATICLIB, ""): QMAKE_PREFIX_STATICLIB = lib
    equals(QMAKE_EXTENSION_STATICLIB, ""): QMAKE_EXTENSION_STATICLIB = a

    PRE_TARGETDEPS *= $$OUT_PWD/$$DESTDIR/../lib/$${QMAKE_PREFIX_STATICLIB}eventdispatcher_epoll$${LIB_SUFFIX}.$${QMAKE_EXTENSION_STATICLIB}

    !CONFIG(console) {
        PRE_TARGETDEPS *= $$OUT_PWD/$$DESTDIR/../lib/$${QMAKE_PREFIX_STATICLIB}eventdispatcher_epoll_qpa$${LIB_SUFFIX}.$${QMAKE_EXTENSION_STATICLIB}
    }
}
else:win32 {
    PRE_TARGETDEPS *= $$OUT_PWD/$$DESTDIR/../lib/eventdispatcher_epoll$${LIB_SUFFIX}.lib
    !CONFIG(console) {
        PRE_TARGETDEPS *= $$OUT_PWD/$$DESTDIR/../lib/eventdispatcher_epoll_qpa$${LIB_SUFFIX}.lib
    }
}

equals(TARGET, tst_qsocketnotifier):win32 {
    LIBS += $$QMAKE_LIBS_NETWORK
}
