/*******************************************************************************
 * @file    joystick.c
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    Apr 29th 2023
 *******************************************************************************/
#include "joystick.h"

/** Global Variables **/
int file_fd;

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int joystick_init()
{
    file_fd = open(JOYSTICK_DEV, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    if (file_fd < 0)
    {
        perror("Error while opening device");
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
void joystick_close()
{
    if (file_fd > 0)
        close(file_fd);
}

/*******************************************************************************
 * @brief
 *
 * @return
 *******************************************************************************/
int joystick_read(struct joystick_data_t *joystick_data)
{
    uint16_t ads1115_data[4];
    char mux_select[2] = "0";
    int ret;

    for (int i = 0; i < 4; i++)
    {
        ret = write(file_fd, mux_select, 2);

        if (ret < 0) 
        {
            perror("Failed to write to ads1115");
            break;
        }

        ret = read(file_fd, &ads1115_data[i], 2);

        if (ret < 0) 
        {
            perror("Failed to read from ads1115");
            break;
        }

        mux_select[0] += 1;
        usleep(20000);
    }

    if (ret >= 0)
    {
        joystick_data->button = ads1115_data[0] < 10 ? 1 : 0;
        joystick_data->y_pos = (((JOYSTICK_Y_DEF - ads1115_data[2]) * 128) / JOYSTICK_Y_MAX);
        joystick_data->x_pos = (((JOYSTICK_X_DEF - ads1115_data[3]) * 128) / JOYSTICK_X_MAX);

#if JOYSTICK_EN_LOGS
        printf("\n");
        for (int i = 0; i < 4; i++)
            printf("%u\t", ads1115_data[i]);

        printf("\n");
        printf("%d\tNA\t%d\t%d\n", joystick_data->button, joystick_data->y_pos, joystick_data->x_pos);
#endif
    }

    return ret;
}

#if JOYSTICK_EN_TEST
int main()
{
    struct joystick_data_t jd;

    joystick_init();

    for (int i = 0; i < 10; i++)
        joystick_read(&jd);

    joystick_close();
}
#endif