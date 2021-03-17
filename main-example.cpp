/******************************************
*main.cpp: example to read data fom the ADC
* uising PIGPIO library
* 
*Changed:   17.033.2021,Andreas Brader
******************************************/
#include "ads1115.h"

int i2cport = 1;
uint16_t ADS_address = 0x48;
bool alert_on_error= true;
                      
int main(int argc, char **argv)
{
    std::cout << "Hello World!" << std::endl;


 // initialise pigpio
 gpioInitialise();

  // create instance of class
  Canalog_read* ADS1115 = new Canalog_read( i2cport, ADS_address, alert_on_error); 
  
  // configure to read channel 0
  uint16_t read_Ch0 =   ADS1115_REG_CONFIG_MUX_SINGLE_0 //channel 0
                      | ADS1115_REG_CONFIG_PGA_2_048V   // max 2,048V
                      | ADS1115_REG_CONFIG_CPOL_ACTVLOW // alert pin active low
                      | ADS1115_REG_CONFIG_CMODE_TRAD   // traditional comperator on alert
                      | ADS1115_REG_CONFIG_DR_16SPS     // 16 samples measured per second
                      | ADS1115_REG_CONFIG_MODE_SINGLE  // singel measurement on poll
                      | ADS1115_REG_CONFIG_OS_SINGLE    // trigger single conversion
                      | ADS1115_REG_CONFIG_CQUE_1CONV;   // asserts alert on one measurement over/under limit
  
  double mes_Result = 0;  // container to write measurement to 
   
                
  while(true)
    {
      mes_Result = ADS1115->ads1115_read(read_Ch0);
      
      std::cout << mes_Result << " milliVolt" << std::endl;
      std::cin.get();
    }
    
  // terminate pigpio  on multiple chrashes please reboot the system!
   gpioTerminate();

   
       return 0;
}
