/* drivers/leds/leds-nuc970.c
 *
 * NUC970 - LEDs GPIO driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_data/leds-nuc970.h>

#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

/* our context */

struct nuc970_gpio_led {
	struct led_classdev		 cdev;
	struct gpio_led	*pdata;
};

static inline struct nuc970_gpio_led *pdev_to_gpio(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}

static inline struct nuc970_gpio_led *to_gpio(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct nuc970_gpio_led, cdev);
}

static void nuc970_led_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct nuc970_gpio_led *led = to_gpio(led_cdev);
	struct gpio_led *pd = led->pdata;
	/*int state = (value ? 1 : 0) ^ (pd->flags & NUC970_LEDF_ACTLOW);*/
  int state = (value ? 1: 0);

	/* there will be a short delay between setting the output and
	 * going from output to input when using tristate. */

  pr_notice("nuc970_led_set: GPIO%d = %d\n", pd->gpio, state);
	gpio_set_value(pd->gpio, state);

	/*
  if (pd->flags & NUC970_LEDF_TRISTATE) {
		if (value)
			gpio_direction_output(pd->gpio, state);
		else
			gpio_direction_input(pd->gpio);
	}
  */
}

static int nuc970_led_remove(struct platform_device *dev)
{
	struct nuc970_gpio_led *led = pdev_to_gpio(dev);

	led_classdev_unregister(&led->cdev);

	return 0;
}

static int nuc970_led_probe(struct platform_device *dev)
{
  struct gpio_led_platform_data *pdata_a = dev->dev.platform_data;
	struct gpio_led *pdata;
	struct nuc970_gpio_led *led;
	int ret;
  
  pdata = &(pdata_a->leds[0]);

	led = devm_kzalloc(&dev->dev, sizeof(struct nuc970_gpio_led),
			   GFP_KERNEL);
	if (led == NULL) {
    dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
  }
  
  pr_notice("nuc970_led_probe\n");
  
	platform_set_drvdata(dev, led);

	led->cdev.brightness_set = nuc970_led_set;
	led->cdev.default_trigger = pdata->default_trigger;
	led->cdev.name = pdata->name;
	led->cdev.flags |= LED_CORE_SUSPENDRESUME;

	led->pdata = pdata;

	ret = devm_gpio_request(&dev->dev, pdata->gpio, "NUC970_LED");
	if (ret < 0)
		return ret;

	/* no point in having a pull-up if we are always driving */

	//s3c_gpio_setpull(pdata->gpio, S3C_GPIO_PULL_NONE);

  pr_notice("leds-nuc970: set(%s)(%d) output\n", pdata->name, pdata->gpio);
	gpio_direction_output(pdata->gpio, 1);
  
	/* register our new led device */

	ret = led_classdev_register(&dev->dev, &led->cdev);
	if (ret < 0)
		dev_err(&dev->dev, "led_classdev_register failed\n");

	return ret;
}

static struct platform_driver nuc970_led_driver = {
	.probe		= nuc970_led_probe,
	.remove		= nuc970_led_remove,
	.driver		= {
		.name		= "leds-nuc970",
    .owner  = THIS_MODULE,
	},
};

module_platform_driver(nuc970_led_driver);

MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_DESCRIPTION("NUC970 LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-nuc970");
