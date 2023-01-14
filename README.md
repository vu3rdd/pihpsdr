# pihpsdr

Fork of John Melton G0ORX's pihpsdr.

## Related repositories

* [Radioberry controller firmware](https://github.com/vu3rdd/rb_controller)
* [R-Pi4 configuration files for the controller hardware](https://github.com/vu3rdd/radioberry-controller-pi-config)

## IMPORTANT

If you encounter any error, copy the error and paste it into your
favourite search engine and try to understand the problem. Ham radio
is all about experimentation and self education.

# logging via a program on another computer (via network)

The idea is that `rigctld` would connect to the radio via network. All
other programs would connect to `rigctld`.

0. Launch `rigctld` to connect to the tcp port of pihpsdr:

```
rigctld -m 2040 -r $IPADDR:19090
```

where `$IPADDR` is the ip address of the computer running pihpsdr.

1. create a virtual pair of serial port.

```
sudo socat pty,link=/dev/vtty0 pty,link=/dev/vtty1
```

Here `vtty0` and `vtty1` are regular files and can be created anywhere
in the filesystem. If it is in a non-privileged location, then `sudo`
is not needed.

2. run the ts-2000 rigctl emulation program `rigctlcom` so that it is
   listening for TS-2000 commands on one end of the above serial port
   parts and talks to rigctld natively on the other. On the other end,
   you would connect a logging program or another program that expects
   a serial/COM port.
   
```
sudo rigctlcom  -m 2 -R /dev/vtty0 -S 115200
```

3. Now, as a test, open minicom, turn off hardware control off, set
   the port to `/dev/vtty1` and type in `FA;`. It should display the
   current frequency used by pihpsdr. Other programs like logging
   programs can now connect to it.


This is the basic idea. One can run all of these programs in a single
computer or in multiple computers connected via network with the right
configuration. `rigctl` and `rigctlcom` also takes a `-r` switch to
specify the address of of the "NET rigctl" as the radio, provided a
`-m 2` parameter is also passed to rigctl. Some knowledge of how the
programs all work together is helpful when debugging issues.

## Original readme

Raspberry Pi 3/4 standalone code for HPSDR

Supports both the old and new ethernet protocols.

See the Wiki (https://github.com/g0orx/pihpsdr/wiki) for more information about building and running piHPSDR.

Note: The latest source now code has the gpiod branch merged in and also reuqires the latest version of wdsp.
