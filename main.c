
#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "math.h"
#include "ADS7128.h"
#include "hardware/adc.h"

uint8_t recent_read[2];

int main() {
    stdio_init_all();
    
    sleep_ms(3000);


    /******Setup external ADC ADS1728******/
    ads7128_i2c_init(i2c1, 19, 18);
    
    ads7128_config_osr(OSR_128_SAMPLES);

    ads7128_calibrate();

    ads7128_channel_cfg_gpio(AN_CH1, GPIO_OUTPUT);
    ads7128_channel_cfg_gpio(AN_CH2, GPIO_INPUT);
  

    sleep_ms(250);

    printf("Going into the While Loop\n");

    while (1) {


        uint16_t ch0_read = ads7128_read_channel_analog(AN_CH0);
        //uint16_t ch1_read = ads7128_read_channel_analog(AN_CH1);
        //uint16_t ch2_read = ads7128_read_channel_analog(AN_CH2);
        ads7128_write_channel_gpio(AN_CH1, OUTPUT_LOW);
        uint8_t ch2_read = ads7128_read_channel_gpio(AN_CH2);
        printf("Ch0 %d Ch2 %d\n", ch0_read, ch2_read);
        sleep_ms(10);
        ads7128_write_channel_gpio(AN_CH1, OUTPUT_HIGH);
        ch2_read = ads7128_read_channel_gpio(AN_CH2);
        printf("Ch1 %d Ch2 %d\n", ch0_read, ch2_read);

        sleep_ms(100);



    }

}


