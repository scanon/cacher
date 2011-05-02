#!/bin/sh

FILE=/etc/hosts
CACHER=../cacher
LIB=../libtrap.so
TEST=cat

$CACHER -d
$CACHER $FILE

OUT=/tmp/cat.$$

cat $FILE > $OUT.wo
LD_BIND_NOW=1 LD_PRELOAD=$LIB cat $FILE > $OUT.wi

diff $OUT.wo $OUT.wi

if [ $? -eq 0 ] ; then
  echo "$TEST: Passed"
else
  echo "$TEST: Failed"
fi
rm $OUT.wo $OUT.wi

$CACHER -d
