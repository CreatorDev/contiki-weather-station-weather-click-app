
/**
 * @file
 * Helpers functions for error handling.
 *
 * @author Imagination Technologies
 *
 * @copyright Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group
 * companies and/or licensors.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "sys/clock.h"
#include "errors-helper.h"
#include "pic32_gpio.h"

static void ClockDelayMSec(uint16_t howlong)
{
	while(howlong--) clock_delay_usec(1000);
}

static void BlinkErrorLed(int blinkMs, int delayMs)
{
    GPIO_SET(LED2_PORT, LED2_PIN);
    ClockDelayMSec(blinkMs);
    GPIO_CLR(LED2_PORT, LED2_PIN);
    ClockDelayMSec(delayMs);
}

void InformIfErrorAndContinueMsg(AwaError type, char* errorDescription)
{
    if (type == AwaError_Success)
    {
        return;
    }
    if (errorDescription != NULL)
    {
        printf("Error: %s\n", errorDescription);
    }
    int longBlinks = type / 5;
    int shortBlinks = type % 5;
    int t;
    for(t = 0; t < longBlinks; t++)
    {
        BlinkErrorLed(500, 90);
    }
    for(t = 0; t < shortBlinks; t++)
    {
        BlinkErrorLed(250, 90);
    }
    ClockDelayMSec(1000);
}

void StopIfErrorMsg(AwaError type, char* errorDescription)
{
    if (type == AwaError_Success) {
        return;
    }
    while(1)
    {
        InformIfErrorAndContinueMsg(type, errorDescription);
    }
}
