#!/bin/sh

killall pingpong_server
timeout=${timeout:-100}
bufsize=${bufsize:-16384}

for numSessions in 100 1000; do
  for numThreads in 1 2 3 4; do
    sleep 5
    echo "Bufsize: $bufsize Threads: $numThreads Sessions: $numSessions"
    bin/pingpong_server 0.0.0.0 55555 $numThreads & srvpid=$!
    sleep 1
    bin/pingpong_client 127.0.0.1 55555 $numThreads $bufsize $numSessions $timeout
    kill -9 $srvpid
  done
done
