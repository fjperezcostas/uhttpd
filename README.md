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
Time taken for tests:   119.165 seconds
Complete requests:      1000000
Failed requests:        0
Total transferred:      367000000 bytes
HTML transferred:       323000000 bytes
Requests per second:    8391.76 [#/sec] (mean)
Time per request:       11.916 [ms] (mean)
Time per request:       0.119 [ms] (mean, across all concurrent requests)
Transfer rate:          3007.59 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   10  96.1      1    3113
Processing:     0    2  35.4      2   13378
Waiting:        0    1  35.4      1   13377
Total:          0   12 104.2      3   14426

Percentage of the requests served within a certain time (ms)
  50%      3
  66%      3
  75%      3
  80%      3
  90%      4
  95%      4
  98%      6
  99%      9
 100%  14426 (longest request)
```