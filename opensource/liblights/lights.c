/*
 * Copyright (C) 2014 Harsh Panchal <panchal.harsh18@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lights.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>
#include <hardware/hardware.h>

static pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;

const char* const PANEL_BACKLIGHT = "/sys/class/backlight/panel/brightness";
const char* const LED_BACKLIGHT = "/sys/class/leds/button-backlight/brightness";

static int set_backlight(struct light_device_t* dev, struct light_state_t const* state)
{
	int ret;
	pthread_mutex_lock(&locker);
	ret = write_int_to_sysfs(PANEL_BACKLIGHT, convert_to_mono(state));
	pthread_mutex_unlock(&locker);
	return ret;
}

static int set_leds(struct light_device_t* dev, struct light_state_t const* state)
{
	int ret;
	pthread_mutex_lock(&locker);
	if(convert_to_binary(state) == 1)
		ret = write_int_to_sysfs(LED_BACKLIGHT, 0);
	else
		ret = write_int_to_sysfs(LED_BACKLIGHT, 1);
	pthread_mutex_unlock(&locker);
	return ret;
}

static int flush_channel(struct light_device_t *ldevice)
{
	if (!ldevice)
		return -1;
	else
		free(ldevice);
	return 0;
}

static int open_lights(const struct hw_module_t *module, const char *id, struct hw_device_t **device)
{
	pthread_mutex_init(&locker, NULL);
	int(*set_light)(struct light_device_t *dev, struct light_state_t const *state);
	if(strcmp(LIGHT_ID_BACKLIGHT, id) == 0)
		set_light = set_backlight;
	else if(strcmp(LIGHT_ID_BUTTONS, id) == 0)
		set_light = set_leds;
	else
		return -EINVAL;

	struct light_device_t *temp = malloc(sizeof(struct light_device_t));
	memset(temp, 0, sizeof(*temp));

	temp->common.tag = HARDWARE_DEVICE_TAG;
	temp->common.version = 1;
	temp->common.module = (struct hw_module_t *)module;
	temp->common.close = (int (*)(struct hw_device_t *))flush_channel;
	temp->set_light = set_light;

	*device = (struct hw_device_t *)temp;
	return 0;
}

static struct hw_module_methods_t lights_module_methods = 
{
	.open = open_lights,
};

struct hw_module_t HAL_MODULE_INFO_SYM =
{
	.tag = HARDWARE_MODULE_TAG,
	.version_major = 0,
	.version_minor = 1,
	.id = LIGHTS_HARDWARE_MODULE_ID,
	.name = "Janice Lights Module",
	.author = "Harsh Panchal",
	.methods = &lights_module_methods,
};

