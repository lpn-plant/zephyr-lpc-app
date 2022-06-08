/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SLEEP_TIME_MS	1

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */

#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;
//-----------------------
#define LCLK_NODE	DT_ALIAS(pin2lclk)
//#if !LCLK_NODE_HAS_STATUS(LCLK_NODE, okay)
//#error "Unsupported board: pin2lclk devicetree alias is not defined"
//#endif

static const struct gpio_dt_spec pin_lclk = GPIO_DT_SPEC_GET_OR(LCLK_NODE, gpios,
							      {0});

static struct gpio_callback pin_lclk_cb_data;

//-----------------------
#define LRESET_NODE	DT_ALIAS(pin2lreset)
//#if !LCLK_NODE_HAS_STATUS(LCLK_NODE, okay)
//#error "Unsupported board: pin2lclk devicetree alias is not defined"
//#endif

static const struct gpio_dt_spec pin_lreset = GPIO_DT_SPEC_GET_OR(LRESET_NODE, gpios,
							      {0});

static struct gpio_callback pin_lreset_cb_data;

//-----------------------
#define TEST_NODE	DT_ALIAS(pin2test)
//#if !LCLK_NODE_HAS_STATUS(LCLK_NODE, okay)
//#error "Unsupported board: pin2lclk devicetree alias is not defined"
//#endif

static const struct gpio_dt_spec pin_test = GPIO_DT_SPEC_GET_OR(TEST_NODE, gpios,
							      {0});
//----------------------
/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});

volatile int valpin = 0;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void lclk_detected (const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("LCLK clock detected at %" PRIu32 "\n", k_cycle_get_32());
    gpio_pin_toggle(pin_test.port, pin_test.pin);
    //gpio_pin_set_dt(&pin_test, 1);
}


void main(void)
{
	int ret;
    //GPIO pins initialisations

	if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return;
	}
    //---------------------------
     
	if (!device_is_ready(pin_lclk.port)) {
		printk("Error: button device %s is not ready\n",
		       pin_lclk.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&pin_lclk, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, pin_lclk.port->name, pin_lclk.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&pin_lclk,
					      GPIO_INT_EDGE_RISING);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, pin_lclk.port->name, pin_lclk.pin);
		return;
	} 
    //---------------------------

	if (!device_is_ready(pin_test.port)) {
		printk("Error: button device %s is not ready\n",
		       pin_test.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&pin_test, GPIO_OUTPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, pin_test.port->name, pin_lclk.pin);
		return;
	}

    //---------------------------

    //Callbacks from GPIO pins interrupts

	gpio_init_callback(&pin_lclk_cb_data, lclk_detected, BIT(pin_lclk.pin));
	gpio_add_callback(pin_lclk.port, &pin_lclk_cb_data);
	printk("Set up callback at %s pin %d\n", pin_lclk.port->name, pin_lclk.pin);
    
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up callback at %s pin %d\n", button.port->name, button.pin);

	if (led.port && !device_is_ready(led.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}

    //gpio_pin_set_dt(&led, 1);

	printk("Press the button\n");
	if (led.port) {
		while (1) {
			/* If we have an LED, match its state to the button's. */
			int val = gpio_pin_get_dt(&button);

			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
			}
			k_msleep(SLEEP_TIME_MS);
		}
	}
}

