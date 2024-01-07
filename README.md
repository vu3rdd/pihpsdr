# pihpsdr

Fork of John Melton G0ORX's pihpsdr.

# Why a fork?

This fork started as a way to make a table-top radio around the
PA3GSB's Radioberry board. A pico based board was built. The pico
talked CAT commands to control the pihpsdr running on the Raspberry
pi. The original codebase has mostly remained stagnant.

A number of rigctl fixes and a bit of customizations were
done. Another fork by Christoph, DL1YCF gained prominence in the HL2
world. Christoph added more radios, made fixes etc. It is now a
defacto pihpsdr source. If I knew that that another fork would develop
further, I would not have made a fork. But now it is too late. We both
have taken slightly different approach. I have some strong opinions
about the quality of the current codebase and the way it is
structured. I think making big refactoring in the pihpsdr codebase is
hard. There are globals all over the place. This fork has thrown off a
bit of code (soapysdr etc). There is a lot more code that I want to
throw away.

May be this fork has only got some ~10 users perhaps. We hardly
promote it in various mailing lists. But this application is like my
[home cooked
meal](https://www.robinsloan.com/notes/home-cooked-app/). We add
features very carefully and instead focus on long term maintenance and
quality. We are not there yet in terms of quality expectations of the
code, but I have some concrete plans to reach there.

## Related repositories

* [Radioberry controller firmware](https://github.com/vu3rdd/rb_controller)
* [R-Pi4 configuration files for the controller hardware](https://github.com/vu3rdd/radioberry-controller-pi-config)

## IMPORTANT

If you encounter any error, copy the error and paste it into your
favourite search engine and try to understand the problem. Ham radio
is all about experimentation and self education.

# Installation

At the moment, this fork depends on a few libraries for noise
cancellation (NR3 and NR4). I suggest using the installation
[script](https://raw.githubusercontent.com/vu3rdd/radioberry-controller-pi-config/master/pihpsdr_install.sh)
from the above
[radioberry-controller-pi-config](https://github.com/vu3rdd/radioberry-controller-pi-config)
repo. It takes care of installing everything needed for pihpsdr and
pihpsdr itself. It does not install radioberry kernel module and
install the rbf file etc. For that, please use Johan's scripts for
now. Please email me if you encounter any problems installing it.

Ideally these should possibly go into the makefile and the dependent
libraries should be git submodules. I may make that change soon.

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
