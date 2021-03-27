# soil_moisture_wemos

### soil_moisture_wemos.ino

   Main file for the code using Deep Sleep to minimize power consumption.  My original version ran on an Arduino UNO, now runs on an ESP8266 style processor, the ESP8285 specifically (Wemos D1 Mini Lite).
   
   The code is setup for over-the-air updating.  You need to tell the ESP8266 device to stay awake (using the awake command described below) rather than sleeping so you will have enough time to update it.  You can command it to go back to the normal transmit-then-sleep cycle after you have updated it.
   
### secrets_example.h
 
   Set up your secrets.h file like this.  The MYADDR field is the last octet of the IP address. I use a 192.168.2.x network.
   
Connect the Adafruit sensor as described in:

https://learn.adafruit.com/adafruit-stemma-soil-sensor-i2c-capacitive-moisture-sensor

   The sensor draws 5 mA in normal operation.  You can connect the +V pin to D6 on the ESP8266 and the program will turn the sensor on when starting.  It will turn the sensor off when going to sleep.  So instead of 5 mA, the whole setup draws about 0.1 mA.

   In HA "Services" mqtt.publish use this example to set retain flag for this topic

#### {"topic": "sm1/modereq","payload":"awake","retain":true}
 -or-
#### {"topic": "sm1/modereq","payload":"sleep","retain":true}

 
 and switch between going to sleep after a report, or staying 
 awake indefinitely.
 
 See services.png for a different view.
 
   A helpful webpage from randomnerdtutorials:

https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/

another good reference:

https://thingpulse.com/max-deep-sleep-for-esp8266/

