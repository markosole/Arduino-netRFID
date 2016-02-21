#PHP receiver
PHP script that receives RFID device request, search database for users and sends the "answer" back to Arduino.

#Usage
Arduino sends a URL string to this PHP script. URL is defined in Arduino code. PHP file should be placed on web server, or PC running Apache server. 
Example: http://your-domain.com/arduino/index.php
Or in local LAN http://192.168.0.10/arduino/index.php  - example IP of PC where Apache is installed. 

#Version
 - v1.0
