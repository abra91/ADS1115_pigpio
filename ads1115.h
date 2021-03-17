/******************************************
*ads1115.h: This File contains class-
*definition to interface the analog-digital-driver
* 
*Changed:   17.033.2021,Andreas Brader
******************************************/
#ifndef ADS1115_H_INCLUDED
#define ADS1115_H_INCLUDED

#include "ads1115_defs.h"
#include <iostream>                 //std-in-/output-opterations
#include <fstream>
#include <string.h>                 //some c-based string-operations
#include <pigpio.h>                 //header to use the gpio-accessing library(pigpio)
#include <unistd.h>
//#include <bitset>  // may be used for debugging issues         
class Canalog_read 
{
    //Class-Variables
    private:
              //Incrementing counter to generate unique ids
    //Member-Variables
    private:
                            // contains the instance-id
        int ADS_dev_handle;                     // conversation handle
        uint8_t ADS_address;                    // hardware address of device
        uint8_t reg_addr;                       // register address
        uint16_t ADS_config;                    // backup of settings
        bool error_enabled;                     // error flag

        
    //Internal Methodes
    private:  
        std::string errorNum(int eValue);                                       // Error code description
        uint16_t switchByte(uint16_t inWord);

    //External Methodes
    public:
        Canalog_read(uint8_t port,uint8_t address, bool error_enabled);   //Constructor,  creates a connection and initiates the device
        ~Canalog_read(void);                                            //Destructor,   removes a connection
        double ads1115_read(uint16_t  r_settings);    // read data from ADC
        void ads1115_alrt_trh( uint16_t upper_edge, uint16_t lower_edge);      // ADS115 alert pin threshold
        void ads111_mVolt_alrt(uint16_t r_settings, int max_mV, int min_mV); // write threshold in milliVolt
    //External Variables


         
};
#endif // ADS1115_H_INCLUDED
