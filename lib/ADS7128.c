
#include "ADS7128.h"


 // device has default bus address of 0x76
i2c_inst_t *i2c_address_ads;

//
uint8_t previous_channel = 0;
uint8_t osr_enabled = 0;

int ads7128_i2c_init(i2c_inst_t *i2c_address, uint8_t scl_pin, uint8_t sda_pin) {
    i2c_address_ads = i2c_address;
    int status = i2c_init(i2c_address, 100 * 1000);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    printf("SI %d \n", status);

    sleep_ms(10);

    //Reset device on startup
    printf("RESETTING DEVICE \n");
    ads7128_reset();

    //Calibrate device to trim values and account for temp effect
    printf("PERFORMING CALIBRATION \n");
    ads7128_calibrate();

    return 1;
}



/*************** Basic I2c Functions to read and write to the ADS7128 *****************/

/***Read full register value ***/
int ads7128_single_register_read(uint8_t register_read, uint8_t* data){
    uint8_t buffer;
    uint8_t single_register[2] = {SINGLE_READ_FRAME, register_read};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, single_register, 2, true, 50000);
    status = i2c_read_timeout_us(i2c_address_ads, ADDR_ADS, &buffer, 1, false, 50000);
    *data = buffer;
    return status;
}

/***Write to full register value ***/
int ads7128_single_register_write(uint8_t register_write, uint8_t data){

    uint8_t single_register[2] = {SINGLE_WRITE_FRAME, register_write};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, single_register, 2, true, 3000);
    status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, &data, 1, false, 3000);
    return status;
}

/***Write full register value for multiple registers ***/
int ads7128_cont_register_write(uint8_t register_write, uint8_t data, uint8_t number_registers){

    uint8_t single_register[2] = {CONTINIOUS_WRITE_FRAME, register_write};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, single_register, 2, true, 3000);
    status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, &data, 1, false, 3000);
    return status;
}

/***Read full register value for multiple registers ***/
int ads7128_cont_register_read(uint8_t register_read, uint8_t data[], uint8_t number_registers){

    uint8_t cont_register[2] = {CONTINIOUS_READ_FRAME, register_read};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, cont_register, 2, true, 3000);
    status = i2c_read_timeout_us(i2c_address_ads, ADDR_ADS, data, number_registers, false, 3000);
    return status;
}

/***Set a single bit (1) in a specific register ***/
int ads7128_bit_set(uint8_t register_set, uint8_t data){

    uint8_t set_bit[3] = {BIT_SET_FRAME, register_set, data};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, set_bit, 3, false, 3000);
    return status;
}

/***Clear a single bit (0) in a specific register ***/
int ads7128_bit_clear(uint8_t register_clear, uint8_t data){

    uint8_t clear_bit[3] = {BIT_CLEAR_FRAME, register_clear, data};
    int status = i2c_write_timeout_us(i2c_address_ads, ADDR_ADS, clear_bit, 3, false, 3000);
    return status;
}

/***Read a specific register and mask all values except the deisred bit
 * The "bit" variable should be the position of the bit (0 to 7) and not the binary value
 * 0000 0100 the bit position is 3 not 4***/
uint8_t ads7128_read_bit(uint8_t address, uint8_t bit){

    uint8_t register_data;
    ads7128_single_register_read(address, &register_data);
    
    register_data &= (1 << bit);
    uint8_t bit_value = register_data >> bit;

    return bit_value;
}


/*************** ADS7128 FUNCTION DEFINITIONS *****************/


/***Function to configure the averaging (over sampling) 
 * Refer to header files for possible OSR rates***/
int ads7128_config_osr(uint8_t osr_rate){
    int status;
    uint8_t reset_byte = 0xF;

    //if OSR is not required, clear all bits to disable
    //else clear all bits then set the required rate
    if(osr_rate == OSR_DIS){
        ads7128_bit_clear(OSR_CFG_ADDRESS, reset_byte);  
        osr_enabled = 0;
    }else{
        ads7128_bit_clear(OSR_CFG_ADDRESS, reset_byte);
        status = ads7128_bit_set(OSR_CFG_ADDRESS, osr_rate);
        osr_enabled = 1;
    }

    return status;
}


/***Function to select the channel for next ADC conversion
 * The function will get the correct channel hex value then
 * disable all channels before setting the required channel***/
void ads7128_channel_set(uint8_t channel){
    
    //uint8_t channel_enable = ads7128_channel_select(channel);
    ads7128_channel_clear_all();
    ads7128_bit_set(CHANNEL_SEL_ADDRESS, channel);
}

/***Function to clear the channel from adc to prepare for next channel selection***/
void ads7128_channel_clear(uint8_t channel){

    uint8_t channel_clear = ads7128_channel_select(channel);

    ads7128_bit_clear(CHANNEL_SEL_ADDRESS, channel_clear);
}

/***Function to clear all channels before selecting new one to read***/
void ads7128_channel_clear_all(){

    uint8_t reset_byte = 0xF;

    ads7128_bit_clear(CHANNEL_SEL_ADDRESS, reset_byte);
}

/***Function to determine the channel id based on the selected channel***/
int ads7128_channel_select(uint8_t channel){

    uint8_t channel_id;

    switch (channel)
    {
    case AN_CH0:
        channel_id = CH0_RECENT;
        break;
    case AN_CH1:
        channel_id = CH1_RECENT;
        break;
    case AN_CH2:
        channel_id = CH2_RECENT;
        break;
    case AN_CH3:
        channel_id = CH3_RECENT;
        break;
    case AN_CH4:
        channel_id = CH4_RECENT;
        break;
    case AN_CH5:
        channel_id = CH5_RECENT;
        break;
    case AN_CH6:
        channel_id = CH6_RECENT;
        break;
    case AN_CH7:
        channel_id = CH7_RECENT;
        break;
    default:
        break;
    }

    return channel_id;
}

/***Reset Device by setting reset register bit***/
void ads7128_reset(){
   
    //Set RST bit to trigger device reset
    ads7128_bit_set(GENERAL_CFG_ADDRESS, RST);
    sleep_ms(1000);

    //Set CH_RST bit to force all channels to be analog input
    ads7128_bit_set(GENERAL_CFG_ADDRESS, CH_RST);
}

/***Function to start conversion***/
void ads7128_start_conversion(){

    ads7128_bit_set(GENERAL_CFG_ADDRESS, CNVST);
}

/***Function to enable stats (Min, Max, Recent reads)***/
void ads7128_enable_stats(){

    ads7128_bit_set(GENERAL_CFG_ADDRESS, STATS_EN);
}

/***Function to disable stats (Min, Max, Recent reads)***/
void ads7128_disable_stats(){

    ads7128_bit_clear(GENERAL_CFG_ADDRESS, STATS_EN);
}

/***Function to read certian analoge channel***/
int ads7128_read_channel_analog(uint8_t channel){

    uint8_t recent_read[2];
    uint16_t output_data;

    if (previous_channel != channel){
        ads7128_channel_set(channel);
        previous_channel = channel;
    }

    //resetting osr status bit
    ads7128_bit_set(SYSTEM_STATUS_ADDRESS, 0x8);

    //Enable the stats to start updating the min/max/recent values
    ads7128_enable_stats();
    //Force start of conversion
    ads7128_start_conversion();


    uint8_t channel_recent_address = ads7128_channel_select(channel);
    //printf("channel address %x channel %x \n", channel_recent_address, channel);
    if (osr_enabled){

        //Check osr bit to read output data
        ads7128_osr_check();
        //disable stats to prevent data update while reading
        ads7128_disable_stats();
        
        //if OSR is enabled then the ouput data is 16bit
        int status = ads7128_cont_register_read(channel_recent_address, recent_read, 2);
        output_data = (recent_read[1] << 8) | (recent_read[1]);
    }else{

        //disable stats to prevent data update while reading
        ads7128_disable_stats();

        //OSR is not enabled the data output is 12bit
        int status = ads7128_cont_register_read(channel_recent_address, recent_read, 2);
        output_data = (recent_read[1] << 4) | (recent_read[1]);
    }

    return output_data;
}


//Function to check if the OSR calculation is done
uint8_t ads7128_osr_check(){
    
    uint8_t osr_status = 0;

    //check osr bit to be set before reading the output registers
    while (osr_status < 1){

        osr_status = ads7128_read_bit(SYSTEM_STATUS_ADDRESS,OSR_DONE);
    }

    return osr_status;
}


//Function to calibrate the sensor due to temp and volt differences
void ads7128_calibrate(){

    uint8_t calibration_done = 1;

    //SET the CALIB bit in the CONFIG address to initialize calibration
    ads7128_bit_set(GENERAL_CFG_ADDRESS, CALIB);

    //Wait while the calibration is running to proceed
    //This is done by checking the CALIB bit when its reset (0)
    while (calibration_done){

        calibration_done = ads7128_read_bit(GENERAL_CFG_ADDRESS,CALIB_Bit);
        sleep_ms(10);
    }

    printf("Calibration Complete \n");
}

/***Configure a channel to be GPIO input or output instead of analog ***/
void ads7128_channel_cfg_gpio(uint8_t channel, uint8_t gpio_type){
    
    //Clear bit that forces all channels to be analog input
    ads7128_bit_clear(GENERAL_CFG_ADDRESS, CH_RST);

    //Set the selected channel as gpio input/output
    ads7128_bit_set(PIN_CFG_ADDRESS, (1<<channel));

    if(gpio_type == GPIO_OUTPUT){
        //set channel as output
        ads7128_bit_set(GPIO_CFG_ADDRESS, (1<<channel));
        //set the output type as push pull
        ads7128_bit_set(GPO_DRIVE_CFG_ADDRESS, (1<<channel));

    }else if(gpio_type == GPIO_INPUT){

        ads7128_bit_clear(GPIO_CFG_ADDRESS, (1<<channel));
    }

}

/***Read channel that is configured into a digital gpio ***/
uint8_t ads7128_read_channel_gpio(uint8_t channel){

    uint8_t read_gpo_channel = ads7128_read_bit(GPI_VALUE_ADDRESS, (channel));

    return read_gpo_channel;

}

/***Write toa  channel that is configured into a digital gpio ***/
void ads7128_write_channel_gpio(uint8_t channel, uint8_t output){
    
    if(output == OUTPUT_HIGH){
        ads7128_bit_set(GPO_VALUE_ADDRESS, (1<<channel));
    }else if(output == OUTPUT_LOW){
        ads7128_bit_clear(GPO_VALUE_ADDRESS, (1<<channel));
    }
    
}