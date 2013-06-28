#! /bin/sh

set -e

/home/vladimir/Qt5.0.2/5.0.2/gcc_64/bin/qmake
make
cd tests
for i in ./tst_*; do ./$i; done
