#!bash
if [[ $1 = "" ]]; then
	exit
else
	PID=$1
fi
if [[ $2 = "" ]]; then
	SLEEP_TIME=30
else
	SLEEP_TIME=$2
fi

if [ ! -d "./perf" ]; then
	mkdir perf
fi

cd perf
perf record -F 99 -p $PID -g -- sleep $SLEEP_TIME
perf script >out.perf
stackcollapse-perf out.perf >out.folded
flamegraph out.folded >perf.svg
