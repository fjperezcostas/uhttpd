# uhttpd #

A micro HTTP web server written in C for Linux systems. It can handle up to 10 concurrent connections at same time.

## Clone and compile the project ##

`git clone https://github.com/fjperezcostas/uhttpd.git && cd uhttpd/ && make`

## Start the web server ##

`./uhttpd`

## Build and run docker image ##

`docker build -t local/utthpd . && docker run --rm -p 8080:8080 --memory="256m" --cpus="1.0" local/uhttpd`

## Supported MIME types ##

```
text/plain
text/html
text/css
text/javascript
application/xml
application/json
image/png
image/jpg
application/octet-stream
```

## Load Tests Results ##

```
ab -n 1000000 -c 100 http://localhost:8080/
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 100000 requests
Completed 200000 requests
Completed 300000 requests
Completed 400000 requests
Completed 500000 requests
Completed 600000 requests
Completed 700000 requests
Completed 800000 requests
Completed 900000 requests
Completed 1000000 requests
Finished 1000000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            8080

Document Path:          /
Document Length:        323 bytes

Concurrency Level:      100
Time taken for tests:   196.196 seconds
Complete requests:      1000000
Failed requests:        1
   (Connect: 0, Receive: 0, Length: 1, Exceptions: 0)
Total transferred:      366999633 bytes
HTML transferred:       322999677 bytes
Requests per second:    5096.95 [#/sec] (mean)
Time per request:       19.620 [ms] (mean)
Time per request:       0.196 [ms] (mean, across all concurrent requests)
Transfer rate:          1826.74 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       9
Processing:     0   19 210.0      0  108282
Waiting:        0   19 179.9      0   53498
Total:          0   20 210.0      1  108283

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      2
  98%      3
  99%   1019
 100%  108283 (longest request)
```