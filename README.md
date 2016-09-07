#Weather click application

This application is a part of [wather station]() project.

app uses [weather click](http://www.mikroe.com/click/weather/) to measure temperature, humidity and air pressure. It connects to Creator Device Server using [AwaLWM2M]() allowing REST clients to read measured values.

Measurements are done periodically with 60 seconds delay.



How To Compile

Assuming you have creator-contiki source code with directory constrained-os 

Edit Makefile and set $(CONTIKI) variable to point to your contiki source code

To build with TI CC2520 6lowpan driver
```
$ make installDependencies
$ make TARGET=mikro-e USE_CC2520=1
```

To build with Cascoda CA8210 6lowpan driver
```
$ make installDependencies
$ make TARGET=mikro-e USE_CA8210=1
```

This will generate hex file which can be flashed onto the MikroE clicker.
