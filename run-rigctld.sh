#!/bin/sh
# rigctld connects to pihpsdr and listens on port 4532
# rigctl clients in the network can connect to this host on port 4532 
rigctld -m 2040 -r localhost:19090 &


