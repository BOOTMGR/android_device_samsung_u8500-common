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

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <cutils/log.h>

#include <hardware/lights.h>
#include <hardware/hardware.h>

#define LOG_TAG "LIGHTS_HAL"

/*
 *  Simple function to write integer value to sysfs entries
 *  returns 0 on success & associated errno on failure
 */
static int write_int_to_sysfs(char const *path, int val)
{
	int fd, nos;
	char buf[10];
	fd = open(path, O_WRONLY);
	if(fd < 0) {
		ALOGE("%s failed to open %s...\n", __FUNCTION__, path);
		return -errno;
	}
	nos = sprintf(buf, "%d\n", val);
	if(write(fd, buf, nos) > 0) {
		return 0;
	} else {
		ALOGE("%s write failed to %s...\n", __FUNCTION__, path);
		return -errno;
	}
}

/*
 *  Janice only supports brightness controlling.
 *  If you can only do a brightness ramp, then use this formula: 
 *  unsigned int brightness = ((77*((color>>16)&0x00ff)) + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
 */
static int convert_to_mono(struct light_state_t const *state)
{
	int color = state->color & 0x00ffffff;
	return ((77 * ((color >> 16) & 0x00ff)) + (150 * ((color >> 8) & 0x00ff)) + (29 * (color & 0x00ff))) >> 8;
}

/*
 *  Janice don't have color leds, all we can do is to enable/disable it (we can change brightness but it is
 *  meaningless for leds, and android doesn't support it) so we need to convert color data in ARGB to format
 *  to its binary equivalent
 */
static int convert_to_binary(struct light_state_t const *state)
{
	return state->color & 0x00ffffff;
}

