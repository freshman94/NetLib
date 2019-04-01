#!/bin/sh

killall pingpong_server
timeout=${timeout:-100}
bufsize=${bufsize:-16384}
numThreads=1

for numSessions in 1 10 100 1000 10000; do
  sleep 5
  echo "Bufsize: $bufsize Threads: $numThreads Sessions: $numSessions"
  taskset -c 0 bin/pingpong_server 0.0.0.0 33333 $numThreads & srvpid=$!
  sleep 1
  taskset -c 1 bin/pingpong_client 127.0.0.1 33333 $numThreads $bufsize $numSessions $timeout
  kill -9 $srvpid
done

