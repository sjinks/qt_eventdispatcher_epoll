# qt_eventdispatcher_epoll [![Build Status](https://secure.travis-ci.org/sjinks/qt_eventdispatcher_epoll.png)](http://travis-ci.org/sjinks/qt_eventdispatcher_epoll)

epoll()-based event dispatcher for Qt (for Linux only)

## Features
* very fast :-)
* compatibility with Qt4 and Qt 5
* does not use any private Qt headers
* passes Qt 4 and Qt 5 event dispatcher, event loop, timer and socket notifier tests

## Unsupported Features
* undocumented `QCoreApplication::watchUnixSignal()` is not supported (GLib dispatcher does not support it either; this feature was removed from Qt 5 anyway)

## Requirements
* Linux kernel >= 2.6.27
* glibc >= 2.9
* Qt >= 4.8.0 (may work with an older Qt but this has not been tested)


## Build

```
cd src
qmake
make
```

The above commands will generate the static library and `.prl` file in `../lib` directory.


## Install

After completing Build step run

```
sudo make install
```

This will install `eventdispatcher_epoll.h` to `/usr/include`, `libeventdispatcher_epoll.a` and `libeventdispatcher_epoll.prl` to `/usr/lib`, `eventdispatcher_epoll.pc` to `/usr/lib/pkgconfig`.


## Usage (Qt 4)

Simply include the header file and instantiate the dispatcher in `main()`
before creating the Qt application object.

```c++
#include "eventdispatcher_epoll.h"

int main(int argc, char** argv)
{
    EventDispatcherEPoll dispatcher;
    QCoreApplication app(argc, argv);

    // ...

    return app.exec();
}
```

And add these lines to the .pro file:

```
CONFIG    += link_pkgconfig
PKGCONFIG += eventdispatcher_epoll
```

or

```
HEADERS += /path/to/eventdispatcher_epoll.h
LIBS    += -L/path/to/library -leventdispatcher_epoll
```


## Usage (Qt 5)

Simply include the header file and instantiate the dispatcher in `main()`
before creating the Qt application object.

```c++
#include "eventdispatcher_epoll.h"

int main(int argc, char** argv)
{
    QCoreApplication::setEventDispatcher(new EventDispatcherEPoll);
    QCoreApplication app(argc, argv);

    // ...

    return app.exec();
}
```

And add these lines to the .pro file:

```
CONFIG    += link_pkgconfig
PKGCONFIG += eventdispatcher_epoll
```

or

```
HEADERS += /path/to/eventdispatcher_epoll.h
LIBS    += -L/path/to/library -leventdispatcher_epoll
```

Qt 5 allows to specify a custom event dispatcher for the thread:

```c++
QThread* thr = new QThread;
thr->setEventDispatcher(new EventDispatcherEPoll);
```
