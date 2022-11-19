#!/bin/sh

sudo socat pty,link=/dev/vtty,raw tcp:localhost:19090 &
sudo rigctld -m 2040 -r /dev/vtty &
