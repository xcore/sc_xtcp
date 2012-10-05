# TCP/IP component test script

BINARY=$1
FAILED=false
DIR=`pwd`

if [ "$BINARY" = "" ]; then
	echo "Usage: $0 [test binary]"
	exit
fi

# Start device 
exe=$BINARY libpath=$LD_LIBRARY_PATH xterm -hold -ls -e 'export LD_LIBRARY_PATH=$libpath; xrun --io $exe 2>&1 | tee xrun.out' &

# Run host 2 test
xterm -hold -e "ssh colin 'cd $DIR; ./tcpip_test.py 2'" &

# Run host 1 test
sleep 10; ./tcpip_test.py 2>&1 | tee host.out

while lsof xrun.out; do
	echo waiting
	sleep 1
done

FAILURES=`grep -n -B 5 FAILED host.out xrun.out`
if [ "$FAILURES" != "" ]; then
	FAILED=true
	echo output: $FAILURES
fi

PASSES=`grep -ch PASSED xrun.out`
if [ "$PASSES" != "8" ]; then
	FAILED=true
	echo $PASSES tests passed.
fi

if $FAILED; then
	echo FAILED
else
	echo PASSED
fi


echo $FAILURES
