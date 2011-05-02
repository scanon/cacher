#!/bin/sh

FILE=/etc/hosts
CACHER=../cacher
LIB=../libtrap.so
TEST=seek

$CACHER -d
$CACHER $FILE

OUT=/tmp/seek.$$
ERR=/tmp/err.$$

./seek_test $FILE > $OUT.wo 2> $ERR.wo
LD_BIND_NOW=1 LD_PRELOAD=$LIB \
./seek_test $FILE > $OUT.wi 2> $ERR.wi

diff $OUT.wo $OUT.wi
if [ $? -eq 0 ] ; then
  echo "$TEST: output: Passed"
else
  echo "$TEST: output: Failed"
fi

diff $ERR.wo $ERR.wi
if [ $? -eq 0 ] ; then
  echo "$TEST: returns: Passed"
else
  echo "$TEST: returns: Failed"
fi

rm $OUT.wo $OUT.wi
rm $ERR.wo $ERR.wi

$CACHER -d
