#!/bin/sh

# 20130920 WIP laurent

sudo ../../glougloud -Dvv &
sleep 4
nc -v 127.0.0.1 4431 > test_redis_cli_tcp.log
sudo /usr/local/bin/redis-cli -s /var/lib/glougloud/chroot/socket/redis.sock select 1
sudo /usr/local/bin/redis-cli -s /var/lib/glougloud/chroot/socket/redis.sock sadd /N/192.168.1.254 1
