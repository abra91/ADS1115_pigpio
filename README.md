# ADS1115_pigpio
Using pigpio on a rapberry to read from Texas Instruments 16-Bit analog digital converter ADS1115(tested) or similar IC-s via I2C

### Info
* [ADS1115 16-bit ADC IC](https://www.ti.com/product/ADS1115)
* [ADS1115 16-bit ADC Adafruit Board](https://www.adafruit.com/product/1085)
* [pigpio web](http://www.abyz.me.uk/rpi/pigpio/download.html)
* [pigpio github](https://github.com/joan2937/pigpio/)

### Build and execute
To run the sample program build with g++ int the terminal:

  `cd /"Repository location" `

 executable as example named : ReadAnalog    
  `g++ -Wall  -pthread -o ReadAnalog  *.cpp -lpigpio -lrt`

To execute (need to be sudo as you ):

 `sudo ./ReadAnalog`


