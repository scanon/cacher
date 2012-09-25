#!/bin/sh

for t in $(ls *.t) ; do
  ./$t
done
