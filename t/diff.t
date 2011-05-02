#!/bin/sh

FILE=/etc/hosts
CACHER="../cacher"
LIB="../libtrap.so"
TEST=diff

$CACHER -d
$CACHER $FILE

FILE2=/tmp/diff.$$

cp $FILE $FILE2
LD_BIND_NOW=1 LD_PRELOAD=$LIB diff $FILE $FILE2

if [ $? -eq 0 ] ; then
  echo "$TEST: Passed"
else
  echo "$TEST: Failed"
fi
rm $FILE2

$CACHER -d
