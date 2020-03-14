You can replace your HEG.h and WiFi_API.h with these as well as add the library to your Arduino Libraries folder.

To get the ADS1115 library working (use the addicore version on github), you must also have the I2C device library by jrwoberg.
For Arduino, you must drag the files "I2Cdev.cpp" and "I2Cdev.h" from i2cdevlib-master/Arduino/I2Cdev to the main i2cdevlib-master folder so Arduino can find it.

In I2Cdev.cpp you must add these lines just after #include "I2Cdev.h" at the top:

#ifdef ARDUINO_ARCH_ESP32
    #define BUFFER_LENGTH I2C_BUFFER_LENGTH
#endif
