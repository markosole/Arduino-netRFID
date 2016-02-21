# Arduino-netRFID
Arduino Network based RFID reader. Access control. 
 - Version: v4.0

#Idea
Idea is to build Arduino based RFID reader with User managable access control.
This device is built using Arduino Mega2560, Arduino Network W5100 board, RC522 Serial RFID reader. 
Web based management is used for user ID (tag) management. Web application can be installed on any web server (shared hosting, Linux OS and Windows PC using LAMP stack).

#Part list
 1. Arduino Mega 2560 http://goo.gl/7SypKd
 2. Arduino Network board W5100 http://goo.gl/l6vCAk
 3. RC522 RFID reader http://goo.gl/9oQa7S
 4. 6 Color LED indicator matrix  http://goo.gl/cmXYI3
 

#Arduino software
I have used original code from https://github.com/omersiar/RFID522-Door-Unlock with hard modifications. 
I modified  EEPROM version and added code for Network communication with server.

It is still in development stage, not yet ready for production. 

#Server software
Web PHP application is based on Fuel CMS - CodeIgniter PHP framework. http://www.getfuelcms.com/
I have added functionality and Modules for RFID management. 
All tags and cards ID's are stored in database, managed in one place. There can be unlimited number of card readers installed at location. 
