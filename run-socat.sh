#!/bin/sh

sudo socat pty,link=/dev/vtty,raw tcp:localhost:19090
