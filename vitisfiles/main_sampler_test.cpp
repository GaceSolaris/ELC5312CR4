/*****************************************************************//**
 * @file main_sampler_test.cpp
 *
 * @brief Basic test of nexys4 ddr mmio cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

// #define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "spi_core.h"
#include "math.h"

/**
 * Reconfigured the original function to calculate the angle
   provided by the accelerometer and then turn on an LED to 
   indicate the angle.
 */

void gsensor_check(SpiCore *spi_p, GpoCore *led_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t PART_ID_REG = 0x02;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   float x, y, z, angle;
   int id;

   spi_p->set_freq(400000);
   spi_p->set_mode(0, 0);
   // check part id
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(PART_ID_REG);  // part id address
   id = (int) spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   uart.disp("read ADXL362 id (should be 0xf2): ");
   uart.disp(id, 16);
   uart.disp("\n\r");
   // read 8-bit x/y/z g values once
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //
   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   x = (float) xraw / raw_max;
   y = (float) yraw / raw_max;
   z = (float) zraw / raw_max;
   uart.disp("x/y/z axis g values: ");
   uart.disp(x, 3);
   uart.disp(" / ");
   uart.disp(y, 3);
   uart.disp(" / ");
   uart.disp(z, 3);
   uart.disp("\n\r");

   //Calculates the angle and converts it to degrees
   angle = atan2(y, x) * (180.0/M_PI);
   //Normalizes the angle to start at 0.
   angle = angle + 180;
   //Outputs the value to the serial bus for testing   
   uart.disp("angle: ");
   uart.disp(angle, 3);
   uart.disp("\n\r");   

   
   if (angle >= 315 || angle < 45)
   {
       //Case for 180 degrees (Board is switches at bottom)
       led_p->write(1, 1);
       led_p->write(0, 2);
       led_p->write(0, 3);
       led_p->write(0, 0);       
   }
   else if (angle >= 45 && angle < 135)
   {
       //Case for 270 degrees (Board is switches on right)
       led_p->write(1, 2);
       led_p->write(0, 3);
       led_p->write(0, 0);
       led_p->write(0, 1);       
   }
   else if (angle >= 135 && angle < 225)
   {
       //Case for 0 degrees (Board is switches at top)
       led_p->write(1, 3);
       led_p->write(0, 0);
       led_p->write(0, 1);
       led_p->write(0, 2);       
   }
   else if (angle >= 225 && angle < 315)
   {
       //Case for 90 degrees (Board is switches on left)
       led_p->write(1, 0);
       led_p->write(0, 1);
       led_p->write(0, 2);
       led_p->write(0, 3);       
   }
   else {
       //Case for error values       
       led_p->write(0, 1);
       led_p->write(0, 2);
       led_p->write(0, 3);
       led_p->write(0, 0);
   }
}

GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
PwmCore pwm(get_slot_addr(BRIDGE_BASE, S6_PWM));
DebounceCore btn(get_slot_addr(BRIDGE_BASE, S7_BTN));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));


int main() {
   while (1) {
      gsensor_check(&spi, &led);
      sleep_ms(1000);
   } //while
} //main

