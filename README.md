push model code is inside `/push` folder

pull model code is available in files `broker.cpp` , `publisher.cpp` , `subscriber.cpp`

## Make Sure you have these things installed on your machine

1. Apache Thrift
2. Redis Server
3. redis-plus-plus
4. cmake

## use this command to generate thrift cpp files

thrift --gen cpp pubsub.thrift
