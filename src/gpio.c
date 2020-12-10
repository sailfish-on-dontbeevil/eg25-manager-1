/*
 * Copyright (C) 2020 Arnaud Ferraris <arnaud.ferraris@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gpio.h"

#define GPIO_CHIP1_LABEL "1c20800.pinctrl"
#define GPIO_CHIP2_LABEL "1f02c00.pinctrl"

#define MAX_GPIOCHIP_LINES 352

enum {
    GPIO_OUT_DTR = 0,
    GPIO_OUT_PWRKEY,
    GPIO_OUT_RESET,
    GPIO_OUT_APREADY,
    GPIO_OUT_DISABLE,
    GPIO_OUT_COUNT
};

static unsigned gpio_idx_bh[] = {
    358,
    35,
    68,
    231,
    232
};

static unsigned gpio_idx_ce[] = {
    34,
    35,
    68,
    231,
    232
};

int gpio_sequence_poweron(struct EG25Manager *manager)
{
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_PWRKEY], 1);
    sleep(1);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_PWRKEY], 0);

    g_message("Executed power-on sequence");

    return 0;
}

int gpio_sequence_shutdown(struct EG25Manager *manager)
{
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_RESET], 1);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_DISABLE], 1);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_PWRKEY], 1);
    sleep(1);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_PWRKEY], 0);

    g_message("Executed power-off sequence");

    return 0;
}

int gpio_sequence_suspend(struct EG25Manager *manager)
{
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_APREADY], 1);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_DTR], 1);

    g_message("Executed suspend sequence");

    return 0;
}

int gpio_sequence_resume(struct EG25Manager *manager)
{
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_APREADY], 0);
    gpiod_line_set_value(manager->gpio_out[GPIO_OUT_DTR], 0);

    g_message("Executed resume sequence");

    return 0;
}

int gpio_init(struct EG25Manager *manager)
{
    int i, ret;
    unsigned *gpio_idx;

    manager->gpiochip[0] = gpiod_chip_open_by_label(GPIO_CHIP1_LABEL);
    if (!manager->gpiochip[0]) {
        g_critical("Unable to open GPIO chip " GPIO_CHIP1_LABEL);
        return 1;
    }

    if (manager->braveheart) {
        // BH have DTR on the 2nd gpiochip
        manager->gpiochip[1] = gpiod_chip_open_by_label(GPIO_CHIP2_LABEL);
        if (!manager->gpiochip[1]) {
            g_critical("Unable to open GPIO chip " GPIO_CHIP2_LABEL);
            return 1;
        }
        gpio_idx = gpio_idx_bh;
    } else {
        gpio_idx = gpio_idx_ce;
    }

    for (i = 0; i < GPIO_OUT_COUNT; i++) {
        unsigned int offset, chipidx;

        if (gpio_idx[i] < MAX_GPIOCHIP_LINES) {
            offset = gpio_idx[i];
            chipidx = 0;
        } else {
            offset = gpio_idx[i] - MAX_GPIOCHIP_LINES;
            chipidx = 1;
        }

        manager->gpio_out[i] = gpiod_chip_get_line(manager->gpiochip[chipidx], offset);
        if (!manager->gpio_out[i]) {
            g_critical("Unable to get line %d", i);
            return 1;
        }

        ret = gpiod_line_request_output(manager->gpio_out[i], "eg25manager", 0);
        if (ret < 0) {
            g_critical("Unable to request line %d", i);
            return 1;
        }
    }

    return 0;
}

void gpio_destroy(struct EG25Manager *manager)
{
    int i;

    for (i = 0; i < GPIO_OUT_COUNT; i++)
        gpiod_line_release(manager->gpio_out[i]);

    gpiod_chip_close(manager->gpiochip[0]);
    if (manager->gpiochip[1])
        gpiod_chip_close(manager->gpiochip[1]);
}
