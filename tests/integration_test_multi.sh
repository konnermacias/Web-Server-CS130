#!/bin/bash

#cd build
# Build the web server binary
#echo 'BUILDING'
#make clean
#make
#echo "BUILD COMPLETE"

#config for the server. Outputs config to file
echo "http {
  server {
    listen 8070;
    handler echo {
      url /echo;
    }
    server_name 127.0.0.1;
  }
}" > ../tests/integration/integration_test_config

#build the server with the config file
../build/bin/server_main ../tests/integration/integration_test_config & 

sleep 3

#send server a request
echo "SENDING SERVER REQUEST"
curl -i localhost:8070/echo > ../tests/integration/integration_test_response
#curl -i -s GET / HTTP/1.1> /usr/src/projects/git-r-done/tests/integration_test_response
echo "CHECKING REQUEST FOR CORRECTNESS"
#check response of correctness
DIFF=$(diff -w -B ../tests/integration/integration_expected_response ../tests/integration/integration_test_response)
EXIT_STATUS=$? 

if [ "$EXIT_STATUS" -eq 0 ]
then
    echo "SUCCESS: Standard Echo Integration test passed"
else
    echo "FAILED: Standard Echo Integration test failed"
    echo $DIFF
fi 

echo "TESTING MULTIPLE THREADS"

exec 3<>/dev/tcp/localhost/8070
echo -e "GET /echo HTTP/1" >&3

#Testing 404
curl -i localhost:8070/missing > ../tests/integration/integration_test_response2

#check response of correctness
DIFF=$(diff -w -B ../tests/integration/integration_expected_response2 ../tests/integration/integration_test_response2)
EXIT_STATUS=$? 

if [ "$EXIT_STATUS" -eq 0 ]
then
    echo "SUCCESS: 404 Integration test passed"
else
    echo "FAILED: 404 Integration test failed"
    echo $DIFF
fi 


echo "CHECKING SECOND HALF OF ECHO"
echo -e ".1\r\nhost: localhost\r\nConnection: close\r\n\r\n" >&3

GET /echo HTTP/1.1\r\nhost: localhost\r\nConnection: open\r\n\r\n

cat <&3 > ../tests/integration/integration_test_response3

#check response of correctness
DIFF=$(diff -w -B ../tests/integration/integration_expected_response3 ../tests/integration/integration_test_response3)
EXIT_STATUS=$? 

if [ "$EXIT_STATUS" -eq 0 ]
then
    echo "SUCCESS: Multithreaded Echo Integration test passed"
else
    echo "FAILED: Multithreaded Echo Integration test failed"
    echo $DIFF
fi 


echo "SHUTTING DOWN"
# Shutdown the webserver 
pkill server_main

#rm -f ./tests/integration/integration_test_response

# success (0) or failure (1)
exit "$EXIT_STATUS"