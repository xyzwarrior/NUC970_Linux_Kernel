/*
 * NUC970 - LEDs GPIO connector
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __LEDS_NUC970_H
#define __LEDS_NUC970_H

#define NUC970_LEDF_ACTLOW	(1<<0)		/* LED is on when GPIO low */
#define NUC970_LEDF_TRISTATE	(1<<1)		/* tristate to turn off */

struct nuc970_led_platdata {
	unsigned int		 gpio;
	unsigned int		 flags;

	char			*name;
	char			*def_trigger;
};

#endif /* __LEDS_NUC970_H */
