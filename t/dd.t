#!/bin/sh

FILE=/etc/hosts
CACHER=../cacher
LIB=../libtrap.so
TEST=dd

$CACHER -d
$CACHER $FILE

OUT=/tmp/seek.$$

dd status=noxfer if=$FILE of=$OUT.wo bs=1k skip=1 count=1 > /dev/null 2>&1
LD_BIND_NOW=1 LD_PRELOAD=$LIB dd if=$FILE of=$OUT.wi bs=1k skip=1 count=1 > /dev/null 2>&1

diff $OUT.wo $OUT.wi

if [ $? -eq 0 ] ; then
  echo "$TEST: output: Passed"
else
  echo "$TEST: output: Failed"
fi
rm $OUT.wo $OUT.wi

$CACHER -d
