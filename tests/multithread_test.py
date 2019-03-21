import subprocess
import sys
import telnetlib

# start up server
webserver = subprocess.Popen(["./bin/server_main", "../tests/configs/python_server_config"], stdout=subprocess.PIPE)
host = "localhost"
port = 8080

##
## Gathering expected responses for different requests
##

# echo request
request = 'curl -i localhost:8080/echo'
curl_proc = subprocess.Popen(request, stdout=subprocess.PIPE, shell=True)
echo_response = curl_proc.stdout.read().decode('utf-8')

echo_expected_response = (
    'HTTP/1.1 200\r\n'
    'Content-Type: text/plain\r\n'
    'Content-Length: 82\r\n\r\n'
    'GET /echo HTTP/1.1\r\n'
    'Host: localhost:8080\r\n'
    'User-Agent: curl/7.58.0\r\n'
    'Accept: */*\r\n\r\n'
)


# missing request
request = 'curl -i localhost:8080/missing'
curl_proc = subprocess.Popen(request, stdout=subprocess.PIPE, shell=True)
static_response = curl_proc.stdout.read().decode('utf-8')

static_expected_response = (
    'HTTP/1.1 200\r\n'
    'Content-Type: text/plain\r\n'
    'Content-Length: 36\r\n\r\n'
    'Hello world!\n'
    'This is a boring text.'
)


# multi-thread request
multi_expected_response = (
    'HTTP/1.1 200\r\n'
    'Content-Type: text/plain\r\n'
    'Content-Length: 77\r\n\r\n'
    'GET /echo HTTP/1.1\r\n'
    'User-Agent: telnet\r\n'
    'Host: localhost:8080\r\n'
    'Accept: */*\r\n\r\n'
)

multi_miss_expected_response = (
    'HTTP/1.1 404\r\n'
    'Content-Type: text/plain\r\n'
    'Content-Length: 57\r\n\r\n'
    '404: File not found on path. '
    'Please provide correct path.'
)

partial_request_1 = (
    "GET /echo HT"
)

partial_request_2 = (
    "TP/1.1\r\n"
    "User-Agent: telnet\r\n"
    "Host: localhost:8080\r\n"
    "Accept: */*\r\n\r\n"
)

full_missing_request = (
    "GET /konner HTTP/1.1\r\n"
    "User-Agent: telnet\r\n"
    "Host: localhost:8080\r\n"
    "Accept: */*\r\n\r\n"
)


# now send out partial and full requests
tn1 = telnetlib.Telnet(host, port, 5)
tn1.write(partial_request_1)
tn1_response = tn1.read_eager() 

tn2 = telnetlib.Telnet(host, port, 5)
tn2.write(partial_request_1 + partial_request_2)
tn2_response = tn2.read_all()

tn1.write(partial_request_2)
tn1_response = tn1_response + tn1.read_all()

webserver.kill()


##
## Error checks
##

ec = 0

# check echo output and response
print('****************** Let\'s test ******************')
if echo_response != echo_expected_response:
    ec = 1
    print('Echo output does not match')
    print('Expected:\n' + echo_expected_response)
    print('Response:\n' + echo_response)
else:
    print('Echo Success!')

print('**')

# check static
if static_response != multi_miss_expected_response :
    ec = 1
    print('Error output does not match')
    print('Expected:\n' + static_expected_response)
    print('Response:\n' + static_response)
else:
    print('Static Success!')

print('**')

# check multi-threaded output
thread_ec = 0
if tn1_response != multi_expected_response:
    ec = thread_ec = 1
    print('Partial Request failed')
    print("Expected second response:\n" + multi_expected_response)
    print("Second response:\n" + tn1_response)

if tn2_response != multi_expected_response:
    ec = thread_ec = 1
    print('Full Request failed')
    print("Expected first response:\n" + multi_expected_response)
    print("First response:\n" + tn2_response)

if thread_ec == 0:
    print("Multithread Success!")
else:
    print("Multithread Failure...")


print('**')

#
# did we pass?
#
if ec == 0:
    print("INTEGRATION TEST SUCCEEDED")
    sys.exit(ec)
else:
    print('INTEGRATION TEST FAILED')
    sys.exit(ec)
