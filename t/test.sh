#!/bin/sh
#PBS -l mppwidth=256,mppnppn=4,walltime=00:20:00
#PBS -N BLAST 
#PBS -A mpccc
#PBS -V

# This assumes a few things are already in place.

cd $PBS_O_WORKDIR

cleanup(){
  echo "Cleanup"
  ./cacher -c
  ./cacher
#  for i in $(~/seq 1098 1120) ; do
#    ~/ipcrm -M $i
#  done
  exit
}
trap cleanup 2

export DB=/global/scratch/sd/canon/blast_db/IMG-nr.90.7-2-10
export DB=$SCRATCH/blast_db/IMG2.9.nr
#if [ "$0" = "./testw.sh" ] ; then
if [ $# -gt 0 ] ; then
  echo "Preflight"
  CT=$(./cacher|wc -l)
  echo "preload $CT"
  DEBUG_CACHER=1 ./cacher $DB.*
  ./cacher|wc -l
  export LD_PRELOAD=./libtrap.so
  export LD_LIBRARY_PATH=/project/projectdirs/genomes/apps/lib
#fi
#~/genomes/apps/bin/blastall -d $DB -i in2 -p blastx -w 15 -K 10 -e 0.1 |./wc

sleep 3
echo Cleanup
./cacher -c
./cacher
else
  export NODECT=$( qstat  $PBS_JOBID -x|sed 's/.*<mppnodect>//'|sed 's/<.*//'|head -1)
  echo "Launching on $NODECT nodes"
  aprun -n $NODECT -N 1 $0 1
fi
