/*
 * Generic Electric Unicycle firmware
 *
 * Copyright (C) Casainho, 2015.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "gpio.h"
#include "bldc.h"
#include "pwm.h"

#define HALL_SENSORS_MASK ((1 << 0) | (1 << 1) | (1 << 2))

unsigned int bldc_machine_state = BLDC_NORMAL;
static unsigned int _direction = RIGHT;

static unsigned int svm_table_index_a;
static unsigned int svm_table_index_b;
static unsigned int svm_table_index_c;

// Space Vector Modulation PWMs values, please read this blog message:
// http://www.berryjam.eu/2015/04/driving-bldc-gimbals-at-super-slow-speeds-with-arduino/
// Please see file: BLDC_SPWM_Lookup_tables.ods
unsigned int svm_table [36] =
{
  1152,
  1418,
  1677,
  1919,
  1984,
  2024,
  2038,
  2024,
  1984,
  1919,
  1984,
  2024,
  2038,
  2024,
  1984,
  1919,
  1677,
  1418,
  1152,
  885,
  626,
  384,
  319,
  279,
  265,
  279,
  319,
  384,
  319,
  279,
  265,
  279,
  319,
  384,
  626,
  885
};

void apply_duty_cycle (void)
{
  float duty_cycle;

  duty_cycle = pwm_get_duty_cycle ();
  if (duty_cycle <= 0)
  {
    duty_cycle *= -1; // ivert the signal if duty_cycle is negative, ie: -999 * -1 = 999
  }

  // Scale duty_cycle to be [0 <-> 1] and apply it
  duty_cycle /= 1000.0;
  TIM_SetCompare3(TIM1, (svm_table[svm_table_index_a]) * duty_cycle);
  TIM_SetCompare1(TIM1, (svm_table[svm_table_index_b]) * duty_cycle);
  TIM_SetCompare2(TIM1, (svm_table[svm_table_index_c]) * duty_cycle);
}

void commutation_AB (void)
{
  svm_table_index_a = 26;
  svm_table_index_b = 2;
  svm_table_index_c = 14;
}

void commutation_AC (void)
{
  svm_table_index_a = 20;
  svm_table_index_b = 32;
  svm_table_index_c = 8;
}

void commutation_BC (void)
{
  svm_table_index_a = 14;
  svm_table_index_b = 26;
  svm_table_index_c = 2;
}

void commutation_BA (void)
{
  svm_table_index_a = 8;
  svm_table_index_b = 20;
  svm_table_index_c = 32;
}

void commutation_CA (void)
{
  svm_table_index_a = 2;
  svm_table_index_b = 14;
  svm_table_index_c = 26;
}

void commutation_CB (void)
{
  svm_table_index_a = 32;
  svm_table_index_b = 8;
  svm_table_index_c = 20;
}

void commutation_disable (void)
{
  TIM_CtrlPWMOutputs (TIM1, DISABLE); // PWM Output Disable
}

unsigned int get_current_sector (void)
{
  static unsigned int hall_sensors = 0;
  unsigned int sector;
  float duty_cycle;

  hall_sensors = (GPIO_ReadInputData (GPIOA) & (HALL_SENSORS_MASK)); // mask other pins

  // IDENTIFY the sector from hall sensors signals
  //
  //       cba
  //  00000001 == 1
  //  00000011 == 3
  //  00000010 == 2
  //  00000110 == 6
  //  00000100 == 4
  //  00000101 == 5

  //Halls sequence: 6, 5, 2, 3, 1, 4
  switch (hall_sensors)
  {
    case 1:
    sector = 6;
    break;

    case 2:
    sector = 5;
    break;

    case 3:
    sector = 2;
    break;

    case 4:
    sector = 3;
    break;

    case 5:
    sector = 1;
    break;

    case 6:
    sector = 4;
    break;

    default:
    break;
  }

  /* Get the desired rotation direction */
  duty_cycle = pwm_get_duty_cycle ();
  if (duty_cycle >=0)
  {
    bldc_set_direction (RIGHT);
  }
  else
  {
    bldc_set_direction (LEFT);
  }

  /* Increment or decrement sector based on the desired direction */
  if (bldc_get_direction() == RIGHT)
  {
    sector = increment_sector(sector);
  }
  else if (bldc_get_direction() == LEFT)
  {
    sector = decrement_sector(sector);
  }

  return sector;
}

void commutate (void)
{
  volatile unsigned int sector;

  sector = get_current_sector ();
  switch (sector)
  {
    case 1:
    commutation_AB ();
    break;

    case 2:
    commutation_AC ();
    break;

    case 3:
    commutation_BC ();
    break;

    case 4:
    commutation_BA ();
    break;

    case 5:
    commutation_CA ();
    break;

    case 6:
    commutation_CB ();
    break;

    default:
    commutation_disable ();
    break;
  }

  /* Sector has been selected, now apply the duty_cycle */
  apply_duty_cycle ();
}

unsigned int increment_sector (unsigned int sector)
{
  if (sector < 6)
  {
    sector++;
  }
  else // sector = 6
  {
    sector = 1;
  }

  return sector;
}

unsigned int decrement_sector (unsigned int sector)
{
  if (sector > 1)
  {
    sector--;
  }
  else // sector = 1
  {
    sector = 6;
  }

  return sector;
}

void bldc_set_direction (unsigned int direction)
{
  _direction = direction;
}

unsigned int bldc_get_direction (void)
{
  return _direction;
}

void bldc_set_state (unsigned int state)
{
  bldc_machine_state = state;
}

unsigned int bldc_get_state (void)
{
  return bldc_machine_state;
}
