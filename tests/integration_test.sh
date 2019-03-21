#!/bin/bash

#cd ../
# Build the web server binary
#echo 'BUILDING'
#make clean
#make
#echo "BUILD COMPLETE"

#config for the server. Outputs config to file
echo "http {
  server {
    listen 8080;
    handler echo {
      url /echo;
    }
    handler proxy {
      url /proxy1;
      host localhost;
      path /static1/text.txt;
      port 2000;
    }
    server_name 127.0.0.1;
  }
}" > ../tests/integration/integration_test_config_test_1

echo "http {
  server {
    listen 2000;
    handler static {
      location /static_files1;
      url /static1;
    }
  }
}" > ../tests/integration/integration_test_config_test_2

mkdir static
mkdir static_files1
mkdir memes_r_us

cp -r ../memes_r_us ./
cp -r ../static ./

#build the server with the config file
./bin/server_main ../tests/integration/integration_test_config_test_1 &
./bin/server_main ../tests/integration/integration_test_config_test_2 &

sleep 3

echo "TESTING TWO SERVER PROXY HANDLER"
echo "SENDING SERVER REQUEST"
curl -L localhost:8080/proxy1 > ../tests/integration/integration_test_proxy_response_2
#check response of correctness

DIFF=$(diff -w -B ../tests/integration/integration_proxy_expected ../tests/integration/integration_test_proxy_response_2)
EXIT_STATUS=$? 

if [ "$EXIT_STATUS" -eq 0 ]
then
    echo "SUCCESS: Proxy handler integration test passed"
else
    echo "FAILED: Proxy handler integration test failed"
fi 

echo "SHUTTING DOWN"
# Shutdown the webserver 
pkill server_main

# rm -f ./tests/integration/integration_test_response

# success (0) or failure (1)
exit "$EXIT_STATUS"
