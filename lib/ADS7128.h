#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define ADDR_ADS _u(0x10)

/*********** OPCODES FOR COMMUNICATION WITH ADS7128 ***********/ 

//This frame should be sent to the device after sending the i2c address
//to define the type of communication before sending the target register
//and data

#define SINGLE_READ_FRAME       _u(0X10)
#define SINGLE_WRITE_FRAME      _u(0x8)
#define CONTINIOUS_READ_FRAME   _u(0x30)
#define CONTINIOUS_WRITE_FRAME  _u(0x28)
#define BIT_SET_FRAME           _u(0x18)
#define BIT_CLEAR_FRAME         _u(0x20)

/*********** ADS7128 REGISTERS ***********/ 


/* SYSTEM_STATUS REGISTER */
#define SYSTEM_STATUS_ADDRESS   _u(0x00)
//Rgister bit definitions
#define SEQ_STATUS              _u(0X06)
#define I2C_SPEED               _u(0X05)
#define RMS_DONE                _u(0X04)
#define OSR_DONE                _u(0X03)
#define CRC_ERR_FUSE            _u(0X02)
#define CRC_ERR_IN              _u(0X01)
#define BOR                     _u(0X00)

/* GENERAL CONGIG REGISTER */
#define GENERAL_CFG_ADDRESS     _u(0x01)
//Rgister bit definitions
#define RMS_EN                  _u(1<<7)
#define CRC_EN                  _u(1<<6)
#define STATS_EN                _u(1<<5)
#define DWC_EN                  _u(1<<4)
#define CNVST                   _u(1<<3)
#define CH_RST                  _u(1<<2)
#define CALIB                   _u(1<<1)
#define RST                     _u(0x1)

#define CALIB_Bit               _u(0x1)

/* OSR CONGIG REGISTER */
#define OSR_CFG_ADDRESS        _u(0x3)
//Rgister bit definitions
#define OSR_DIS                _u(0x0)
#define OSR_2_SAMPLES          _u(0x1)
#define OSR_4_SAMPLES          _u(0x2)
#define OSR_8_SAMPLES          _u(0x3)
#define OSR_16_SAMPLES         _u(0x4)
#define OSR_32SAMPLES          _u(0x5)
#define OSR_64_SAMPLES         _u(0x6)
#define OSR_128_SAMPLES        _u(0x7)

/* OPMODE_CFG register */
#define OPMODE_CFG_ADDRESS     _u(0x4)



/* PIN_CFG register */
#define PIN_CFG_ADDRESS        _u(0x5)	
//Rgister bit definitions
#define PIN_CFG_AIN0	       _u(0x00)
#define PIN_CFG_AIN1	 	   _u(0x01)										
#define PIN_CFG_AIN2		   _u(0x02)										
#define PIN_CFG_AIN3		   _u(0x03)										
#define PIN_CFG_AIN4		   _u(0x04)							
#define PIN_CFG_AIN5		   _u(0x05)							
#define PIN_CFG_AIN6		   _u(0x06)								
#define PIN_CFG_AIN7           _u(0x07)


/* GPIO CFG registers*/

#define GPIO_CFG_ADDRESS        _u(0x7)
#define GPIO_INPUT              _u(0x0)
#define GPIO_OUTPUT             _u(0x1)
#define GPO_DRIVE_CFG_ADDRESS   _u(0x9)
#define GPO_PUSH_PULL           _u(0x1)

#define GPO_VALUE_ADDRESS       _u(0xB)
#define GPI_VALUE_ADDRESS       _u(0xD)
#define OUTPUT_HIGH             _u(0x1)
#define OUTPUT_LOW              _u(0x0)

/* Channel Select register */
#define CHANNEL_SEL_ADDRESS    _u(0x11)	
//Rgister bit definitions
#define AN_CH0                _u(0X0)
#define AN_CH1                _u(0X1)
#define AN_CH2                _u(0X2)
#define AN_CH3                _u(0X3)
#define AN_CH4                _u(0X4)
#define AN_CH5                _u(0X5)
#define AN_CH6                _u(0X6)
#define AN_CH7                _u(0X7)


/* RECENT_CHx REGISTERS */

//Rgister bit definitions
#define CH0_RECENT             _u(0xA0)
#define CH1_RECENT             _u(0xA2)
#define CH2_RECENT             _u(0xA4)
#define CH3_RECENT             _u(0xA6)
#define CH4_RECENT             _u(0xA8)
#define CH5_RECENT             _u(0xAA)
#define CH6_RECENT             _u(0xAC)
#define CH7_RECENT             _u(0xAE)


/* RMS_CFG REGISTERS */
#define RMS_CFG_ADDRESS        _u(0xC0)

//Rgister bit definitions
#define RMS_CHID_MASK           _u(0xF0)

#define RMS_CH0                _u(0X0)
#define RMS_CH1                _u(0X10)
#define RMS_CH2                _u(0X20)
#define RMS_CH3                _u(0X30)
#define RMS_CH4                _u(0X40)
#define RMS_CH5                _u(0X50)
#define RMS_CH6                _u(0X60)
#define RMS_CH7                _u(0X70)



/**************Defined Functions****************/

int ads7128_i2c_init(i2c_inst_t *i2c_address, uint8_t scl_pin, uint8_t sda_pin);
int ads7128_single_register_read(uint8_t register_read, uint8_t* data);
int ads7128_single_register_write(uint8_t register_write, uint8_t data);
int ads7128_cont_register_write(uint8_t register_write, uint8_t data, uint8_t number_registers);
int ads7128_cont_register_read(uint8_t register_read, uint8_t data[], uint8_t number_registers);
int ads7128_bit_set(uint8_t register_set, uint8_t data);
int ads7128_bit_clear(uint8_t register_clear, uint8_t data);

uint8_t ads7128_read_bit(uint8_t address, uint8_t bit);


/*************** ADS7128 FUNCTION DEFINITIONS *****************/
int ads7128_config_osr(uint8_t osr_rate);
void ads7128_reset();
uint8_t ads7128_osr_check();
void ads7128_channel_cfg_analog(uint8_t channel);
void ads7128_channel_enable(uint8_t channel);
void ads7128_channel_disable(uint8_t channel);
int ads7128_channel_select(uint8_t channel);
void ads7128_start_conversion();
int ads7128_read_channel_analog(uint8_t channel);
void ads7128_calibrate();
void ads7128_channel_clear_all();

void ads7128_channel_cfg_gpio(uint8_t channel, uint8_t gpio_type);
void ads7128_channel_cfg_analog(uint8_t channel);
uint8_t ads7128_read_channel_gpio(uint8_t channel);
void ads7128_write_channel_gpio(uint8_t channel, uint8_t output);