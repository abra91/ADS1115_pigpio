/******************************************
*ads1115.cpp: This File contains tha class
* methods to
* interface the analog-digital-driver
* 
*Changed:   17.033.2021,Andreas Brader
******************************************/
#include "ads1115.h"
#include "ads1115_defs.h"


/***********************************************************
*Constructor of Class Canalog_read
*Parameters:
*   -port   uint8_t    - the i2c-port which is used
*   -address uint8_t - the i2c-address of the connected device
*   -error_enable - bool to throw error
************************************************************/
Canalog_read::Canalog_read(uint8_t port,uint8_t address, bool error_enabled)
{
                                      
    ADS_address = address;
    ADS_dev_handle = -999;

    if((error_enabled > 0 ) && (error_enabled == 1))  this->error_enabled = true;
    else{ this->error_enabled = false; }          // set class flag

    // connect to ADC
    ADS_dev_handle = i2cOpen(port,ADS_address,0);       //defines the file-descriptor of the slave after opening the connection
    usleep(1000 *ADS1115_CONVERSIONDELAY);

    // check ADC answer
    if ((ADS_dev_handle < 0) && error_enabled) throw(std::string)(errorNum(ADS_dev_handle)) ;   // return error string
    else{ std::cout << " ADS - connected  Handle: "<< ADS_dev_handle << std::endl;  }
       


}
/***********************************************************
*Destructor of Class Canalog_read
*Parameters: -none
************************************************************/
Canalog_read::~Canalog_read()
{
   
    if( ADS_dev_handle >= 0) i2cClose(ADS_dev_handle);    //terminates the slave-connection if existing
}
/***********************************************************
*ads1115_read_channel
*reads value from a specific channel
*Parameters:- configuration of 16-bit
*Return:    -measuread voltage in mV
************************************************************ */
double Canalog_read::ads1115_read(uint16_t r_settings)
{                                 
    uint16_t lsbVal = 0;        // reset variables
    uint16_t raw_data = 0;
    double dataOut = 0;
    int AD_stat = -999;
    uint16_t waitConv = 0; 

    // read frequency for poll delay
    if (( r_settings & ADS1115_REG_CONFIG_MODE_SINGLE) >0)
    {
        switch(r_settings & ADS1115_REG_CONFIG_DR_MASK)
        {
         case   ADS1115_REG_CONFIG_DR_8SPS : waitConv = 1000/8; break;      
         case   ADS1115_REG_CONFIG_DR_16SPS : waitConv = 1000/16; break;
         case   ADS1115_REG_CONFIG_DR_32SPS : waitConv = 1000/32; break;
         case   ADS1115_REG_CONFIG_DR_64SPS : waitConv = 1000/34; break;
         case   ADS1115_REG_CONFIG_DR_128SPS: waitConv = 1000/128; break;
         case   ADS1115_REG_CONFIG_DR_250SPS : waitConv = 1000/250; break;  
         case   ADS1115_REG_CONFIG_DR_475SPS : waitConv = 1000/475; break; 
         case   ADS1115_REG_CONFIG_DR_860SPS : waitConv = 1000/860; break;
         default: throw(std::string)("ADS-config-error");
        }
    }
    
    // lsb setting of measurement
    switch (ADS1115_REG_CONFIG_PGA_MASK & r_settings)  // Masks gain section
    {
        case ADS1115_REG_CONFIG_PGA_0_256V : lsbVal = 8; break; // lsb value in micro volt
        case ADS1115_REG_CONFIG_PGA_0_512V : lsbVal = 16; break;
        case ADS1115_REG_CONFIG_PGA_1_024V : lsbVal = 31; break;
        case ADS1115_REG_CONFIG_PGA_2_048V : lsbVal = 63; break;
        case ADS1115_REG_CONFIG_PGA_4_096V : lsbVal = 125; break;
        case ADS1115_REG_CONFIG_PGA_6_144V : lsbVal = 188; break;
        default: throw(std::string)("ADS-config-error");
    }
    
    // configure settings and write to ADC
    r_settings = switchByte(r_settings);        // switch first and second Byte
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_CONFIG, r_settings); // start conversation
    usleep(1000 * ADS1115_CONVERSIONDELAY );                          // conversion delay

    // error handling
    if ((AD_stat<0) && (this->error_enabled))                                       // error handling
    {
        std::cout << "\t - " << errorNum((AD_stat)) << std::endl;       // give out error code
        throw (std::string)("ADS_error");      // return string according to error code
    }



   // read measured data
    raw_data =  i2cReadWordData(ADS_dev_handle, ADS1115_REG_POINTER_CONVERT);    // read a single block from sensor
    usleep(1000 * ADS1115_CONVERSIONDELAY + 1000 * waitConv );           // wait after conversion
    raw_data = switchByte(raw_data); // switch first and second Byte
    

   // calculate output data
   if ( raw_data > (0x7FFF)) 
    {
        dataOut = -(0xFFFF - raw_data) * lsbVal;               // calculate neagitve values
    }
    else
    {
        dataOut = raw_data * lsbVal;                           // result in microVolt
    
    }
    dataOut = dataOut /  1000;                                 // convert to millivolt
    //std::cout << "volt " << dataOut << std::endl;
    return dataOut;
}
/***********************************************************
*ads1115_alrt_trh
*initiates alert ready pin for last configured pin 
*Parameters:
*           - uint16_t upper_edge   max allowed value of register
*           - uint16_t lower_edge   min allowed value of register
*Return:    -returns 0(always)
************************************************************/
 void Canalog_read::ads1115_alrt_trh( uint16_t upper_edge, uint16_t lower_edge)
{   
    
    int AD_stat = -999;
    
    // write lower threshold to register
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_LOWTHRESH, lower_edge);    // set threshold for ALERT pin
    usleep(1000 * ADS1115_CONVERSIONDELAY ); 
    if ((AD_stat<0) && (this->error_enabled))  throw (std::string)("ADS_error");      // return string according to error code
    
    // write uppe threshold to register
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_HITHRESH, upper_edge);    // set threshold for ALERT pin
    usleep(1000 * ADS1115_CONVERSIONDELAY ); 
    if ((AD_stat<0) && (this->error_enabled))  throw (std::string)("ADS_error");      // return string according to error code
    
    
 }
 
/***********************************************************
*ads111_mVolt_alrt
*initiates alert ready pin for last configured pin 
*Parameters:
*           - uint16_t r_settings   to set alert pin measurement for
*           - uint16_t upper_edge   max allowed value of register
*           - uint16_t lower_edge   min allowed value of register
*Return:    -returns 0(always)
************************************************************/
void Canalog_read::ads111_mVolt_alrt(uint16_t r_settings, int max_mV, int min_mV)
{
    
    uint16_t lsbVal;
    int AD_stat = -999;
    
    //write settings to ADC
    r_settings |= ADS1115_REG_CONFIG_OS_SINGLE; // force to perform a conversion
    r_settings = switchByte(r_settings);        // switch first and second Byte
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_CONFIG, r_settings); // start conversation
    usleep(1000 * ADS1115_CONVERSIONDELAY );                          // conversion delay
    
    // lsb setting of measurement
    switch (ADS1115_REG_CONFIG_PGA_MASK & r_settings)  // Masks gain section
    {
        case ADS1115_REG_CONFIG_PGA_0_256V : lsbVal = 8; break; // lsb value in micro volt
        case ADS1115_REG_CONFIG_PGA_0_512V : lsbVal = 16; break;
        case ADS1115_REG_CONFIG_PGA_1_024V : lsbVal = 31; break;
        case ADS1115_REG_CONFIG_PGA_2_048V : lsbVal = 63; break;
        case ADS1115_REG_CONFIG_PGA_4_096V : lsbVal = 125; break;
        case ADS1115_REG_CONFIG_PGA_6_144V : lsbVal = 188; break;
        default: throw(std::string)("ADS-config-error");
    }
    
    // conversion with max Error of LSB value
    uint16_t lower_edge = (1000*min_mV) / lsbVal;  // convert to microvolt, then to bits
    uint16_t upper_edge = (1000*max_mV) / lsbVal;  // convert to microvolt, then to bits
    
    AD_stat = -999;
        
    // write lower threshold to register    
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_LOWTHRESH, lower_edge);    // set threshold for ALERT pin
    usleep(1000 * ADS1115_CONVERSIONDELAY ); 
    if ((AD_stat<0) && (this->error_enabled))  throw (std::string)("ADS_error");      // return string according to error code
    
    // write upper threshold to register
    AD_stat = i2cWriteWordData(ADS_dev_handle, ADS1115_REG_POINTER_HITHRESH, upper_edge);    // set threshold for ALERT pin
    usleep(1000 * ADS1115_CONVERSIONDELAY ); 
    if ((AD_stat<0) && (this->error_enabled))  throw (std::string)("ADS_error");      // return string according to error code
    
  
}

/***********************************************************
*Return error string
*paste error string from value
*Parameters:- error value
*Return:    -error message
************************************************************/
std::string Canalog_read::errorNum(int eValue)
{
    switch (eValue)
    {
    case    -2  : return ("PI_BAD_USER_GPIO");break;
    case	-3	: return("	PI_BAD_GPIO	"); break;
    case	-4	: return("	PI_BAD_MODE	"); break;
    case	-5	: return("	PI_BAD_LEVEL	"); break;
    case	-6	: return("	PI_BAD_PUD	"); break;
    case	-7	: return("	PI_BAD_PULSEWIDTH	"); break;
    case	-8	: return("	PI_BAD_DUTYCYCLE	"); break;
    case	-15	: return("	PI_BAD_WDOG_TIMEOUT	"); break;
    case	-21	: return("	PI_BAD_DUTYRANGE	"); break;
    case	-24	: return("	PI_NO_HANDLE	"); break;
    case	-25	: return("	PI_BAD_HANDLE	"); break;
    case	-35	: return("	PI_BAD_WAVE_BAUD	"); break;
    case	-36	: return("	PI_TOO_MANY_PULSES	"); break;
    case	-37	: return("	PI_TOO_MANY_CHARS	"); break;
    case	-38	: return("	PI_NOT_SERIAL_GPIO	"); break;
    case	-41	: return("	PI_NOT_PERMITTED	"); break;
    case	-42	: return("	PI_SOME_PERMITTED	"); break;
    case	-43	: return("	PI_BAD_WVSC_COMMND	"); break;
    case	-44	: return("	PI_BAD_WVSM_COMMND	"); break;
    case	-45	: return("	PI_BAD_WVSP_COMMND	"); break;
    case	-46	: return("	PI_BAD_PULSELEN	"); break;
    case	-47	: return("	PI_BAD_SCRIPT	"); break;
    case	-48	: return("	PI_BAD_SCRIPT_ID	"); break;
    case	-49	: return("	PI_BAD_SER_OFFSET	"); break;
    case	-50	: return("	PI_GPIO_IN_USE	"); break;
    case	-51	: return("	PI_BAD_SERIAL_COUNT	"); break;
    case	-52	: return("	PI_BAD_PARAM_NUM	"); break;
    case	-53	: return("	PI_DUP_TAG	"); break;
    case	-54	: return("	PI_TOO_MANY_TAGS	"); break;
    case	-55	: return("	PI_BAD_SCRIPT_CMD	"); break;
    case	-56	: return("	PI_BAD_VAR_NUM	"); break;
    case	-57	: return("	PI_NO_SCRIPT_ROOM	"); break;
    case	-58	: return("	PI_NO_MEMORY	"); break;
    case	-59	: return("	PI_SOCK_READ_FAILED	"); break;
    case	-60	: return("	PI_SOCK_WRIT_FAILED	"); break;
    case	-61	: return("	PI_TOO_MANY_PARAM	"); break;
    case	-62	: return("	PI_SCRIPT_NOT_READY	"); break;
    case	-63	: return("	PI_BAD_TAG	"); break;
    case	-64	: return("	PI_BAD_MICS_DELAY	"); break;
    case	-65	: return("	PI_BAD_MILS_DELAY	"); break;
    case	-66	: return("	PI_BAD_WAVE_ID	"); break;
    case	-67	: return("	PI_TOO_MANY_CBS	"); break;
    case	-68	: return("	PI_TOO_MANY_OOL	"); break;
    case	-69	: return("	PI_EMPTY_WAVEFORM	"); break;
    case	-70	: return("	PI_NO_WAVEFORM_ID	"); break;
    case	-71	: return("	PI_I2C_OPEN_FAILED	"); break;
    case	-72	: return("	PI_SER_OPEN_FAILED	"); break;
    case	-73	: return("	PI_SPI_OPEN_FAILED	"); break;
    case	-74	: return("	PI_BAD_I2C_BUS	"); break;
    case	-75	: return("	PI_BAD_I2C_ADDR	"); break;
    case	-76	: return("	PI_BAD_SPI_CHANNEL	"); break;
    case	-77	: return("	PI_BAD_FLAGS	"); break;
    case	-78	: return("	PI_BAD_SPI_SPEED	"); break;
    case	-79	: return("	PI_BAD_SER_DEVICE	"); break;
    case	-80	: return("	PI_BAD_SER_SPEED	"); break;
    case	-81	: return("	PI_BAD_PARAM	"); break;
    case	-82	: return("	PI_I2C_WRITE_FAILED	"); break;
    case	-83	: return("	PI_I2C_READ_FAILED	"); break;
    case	-84	: return("	PI_BAD_SPI_COUNT	"); break;
    case	-85	: return("	PI_SER_WRITE_FAILED	"); break;
    case	-86	: return("	PI_SER_READ_FAILED	"); break;
    case	-87	: return("	PI_SER_READ_NO_DATA	"); break;
    case	-88	: return("	PI_UNKNOWN_COMMAND	"); break;
    case	-89	: return("	PI_SPI_XFER_FAILED	"); break;
    case	-91	: return("	PI_NO_AUX_SPI	"); break;
    case	-92	: return("	PI_NOT_PWM_GPIO	"); break;
    case	-93	: return("	PI_NOT_SERVO_GPIO	"); break;
    case	-94	: return("	PI_NOT_HCLK_GPIO	"); break;
    case	-95	: return("	PI_NOT_HPWM_GPIO	"); break;
    case	-96	: return("	PI_BAD_HPWM_FREQ	"); break;
    case	-97	: return("	PI_BAD_HPWM_DUTY	"); break;
    case	-98	: return("	PI_BAD_HCLK_FREQ	"); break;
    case	-99	: return("	PI_BAD_HCLK_PASS	"); break;
    case	-100	: return("	PI_HPWM_ILLEGAL	"); break;
    case	-101	: return("	PI_BAD_DATABITS	"); break;
    case	-102	: return("	PI_BAD_STOPBITS	"); break;
    case	-103	: return("	PI_MSG_TOOBIG	"); break;
    case	-104	: return("	PI_BAD_MALLOC_MODE	"); break;
    case	-107	: return("	PI_BAD_SMBUS_CMD	"); break;
    case	-108	: return("	PI_NOT_I2C_GPIO	"); break;
    case	-109	: return("	PI_BAD_I2C_WLEN	"); break;
    case	-110	: return("	PI_BAD_I2C_RLEN	"); break;
    case	-111	: return("	PI_BAD_I2C_CMD	"); break;
    case	-112	: return("	PI_BAD_I2C_BAUD	"); break;
    case	-113	: return("	PI_CHAIN_LOOP_CNT	"); break;
    case	-114	: return("	PI_BAD_CHAIN_LOOP	"); break;
    case	-115	: return("	PI_CHAIN_COUNTER	"); break;
    case	-116	: return("	PI_BAD_CHAIN_CMD	"); break;
    case	-117	: return("	PI_BAD_CHAIN_DELAY	"); break;
    case	-118	: return("	PI_CHAIN_NESTING	"); break;
    case	-119	: return("	PI_CHAIN_TOO_BIG	"); break;
    case	-120	: return("	PI_DEPRECATED	"); break;
    case	-121	: return("	PI_BAD_SER_INVERT	"); break;
    case	-124	: return("	PI_BAD_FOREVER	"); break;
    case	-125	: return("	PI_BAD_FILTER	"); break;
    case	-126	: return("	PI_BAD_PAD	"); break;
    case	-127	: return("	PI_BAD_STRENGTH	"); break;
    case	-128	: return("	PI_FIL_OPEN_FAILED	"); break;
    case	-129	: return("	PI_BAD_FILE_MODE	"); break;
    case	-130	: return("	PI_BAD_FILE_FLAG	"); break;
    case	-131	: return("	PI_BAD_FILE_READ	"); break;
    case	-132	: return("	PI_BAD_FILE_WRITE	"); break;
    case	-133	: return("	PI_FILE_NOT_ROPEN	"); break;
    case	-134	: return("	PI_FILE_NOT_WOPEN	"); break;
    case	-135	: return("	PI_BAD_FILE_SEEK	"); break;
    case	-136	: return("	PI_NO_FILE_MATCH	"); break;
    case	-137	: return("	PI_NO_FILE_ACCESS	"); break;
    case	-138	: return("	PI_FILE_IS_A_DIR	"); break;
    case	-139	: return("	PI_BAD_SHELL_STATUS	"); break;
    case	-140	: return("	PI_BAD_SCRIPT_NAME	"); break;
    case	-141	: return("	PI_BAD_SPI_BAUD	"); break;
    case	-142	: return("	PI_NOT_SPI_GPIO	"); break;
    case	-143	: return("	PI_BAD_EVENT_ID	"); break;
    case	-144	: return("	PI_CMD_INTERRUPTED	"); break;
    case	-145	: return("	PI_NOT_ON_BCM2711	"); break;
    case	-146	: return("	PI_ONLY_ON_BCM2711	"); break;
    default         : return(" undefined Error");
   }
}


uint16_t Canalog_read::switchByte(uint16_t inWord)
{

    uint16_t outWord= 0;

    outWord |= (inWord << 8) & 0xFF00;   // shitft first Byte up
    outWord |= (inWord >> 8) & 0xFF;     // shift second Byte down
    return outWord;
}
