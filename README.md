# soil_moisture_wemos

### soil_moisture_wemos.ino

   Main file for the code using Deep Sleep to minimize power consumption.
   
### secrets_example.h
 
   Set up secrets.h file like this.  The MYADDR field is the last octet of the IP address.
   
Connect the Adafruit sensor as described in:

https://learn.adafruit.com/adafruit-stemma-soil-sensor-i2c-capacitive-moisture-sensor

   The sensor draws 5 mA in normal operation.  You can connect the +V pin to D6 on the ESP8266 and the program will turn the sensor on when starting.  It will turn the sensor off when going to sleep.  So instead of 5 mA, the whole setup draws about 0.1 mA.

   In HA "Services" mqtt.publish use this example to set retain flag for this topic

#### {"topic": "sm1/modereq","payload":"awake","retain":true}
 -or-
#### {"topic": "sm1/modereq","payload":"sleep","retain":true}

 
 and switch between going to sleep after a report, or staying 
 awake indefinitely.
 
 See services.png for an better view.
 
   A helpful webpage from our friends at randomnerdtutorials:

https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/

 Ask if you have questions!
  
   
