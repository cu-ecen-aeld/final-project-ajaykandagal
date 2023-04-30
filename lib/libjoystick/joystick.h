/*******************************************************************************
 * @file    joystick.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 29th 2023
 *******************************************************************************/
#ifndef JOYSTICK_H
#define JOYSTICK_H

/** Standard libraries **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

/** Defines  **/
#define DEBUG_LOGS          0

#define JOYSTICK_DEV        ("/dev/ads1115")
#define JOYSTICK_X_DEF      13500
#define JOYSTICK_Y_DEF      13100
#define JOYSTICK_X_MAX      15000
#define JOYSTICK_Y_MAX      15000  

/** User Data Types **/
struct joystick_data_t
{
    int8_t x_pos;
    int8_t y_pos;
    uint8_t button;
};

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int joystick_init();

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void joystick_close();

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int joystick_read(struct joystick_data_t *joystick_data);

#endif // JOYSTICK_H