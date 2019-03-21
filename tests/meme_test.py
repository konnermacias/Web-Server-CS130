import subprocess
import sys
import telnetlib

# start up server
webserver = subprocess.Popen(["./bin/server_main", "../tests/configs/echo_server_config"], stdout=subprocess.PIPE)
host = "localhost"
port = 8080

##
## Gathering expected responses for different requests
##

# echo request
request = 'curl -i localhost:8080/meme/create'
curl_proc = subprocess.Popen(request, stdout=subprocess.PIPE, shell=True)
create_response = curl_proc.stdout.read().decode('utf-8')

create_expected_response = (
    'HTTP/1.1 200\r\n'
)

static_expected_response = (
    'HTTP/1.1 200\r\n'
    'Content-Type: text/plain\r\n'
    'Content-Length: 36\r\n\r\n'
    'Hello world!\n'
    'This is a boring text.'
)

webserver.kill()

##
## Error checks
##

ec = 0

# check echo output and response
print('****************** Let\'s test ******************')
if create_expected_response not in create_response:
    ec = 1
    print('Create output not 200')
    print (create_response)
else:
    print('Create Success!')

print('**')

# did we pass?
#
if ec == 0:
    print("INTEGRATION TEST SUCCEEDED")
    sys.exit(ec)
else:
    print('INTEGRATION TEST FAILED')
    sys.exit(ec)
