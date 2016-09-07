/**
 * @file
 * LightWeightM2M Weather Station - weather click application
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

/***************************************************************************************************
 * Includes
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "dev/common-clicks.h"
#include "lib/pic32_gpio.h"

#include "awa/static.h"

#include "errors-helper.h"

#include "lwm2m-client-device-object.h"
#include "lwm2m-client-ipso-temperature.h"
#include "lwm2m-client-ipso-barometer.h"
#include "lwm2m-client-ipso-humidity.h"

#include "letmecreate/click/weather.h"
#include "letmecreate/core/i2c.h"
/***************************************************************************************************
 * Definitions
 **************************************************************************************************/

//! \{
#define COAP_PORT               (6000)
#define IPC_PORT                (12345)
#define BOOTSTRAP_PORT          "15683"
#define END_POINT_NAME          "WeatherStation-WeatherClick"
//! \}

/***************************************************************************************************
 * Typedefs
 **************************************************************************************************/

/**
 * A structure to contain lwm2m client's options.
 */
typedef struct
{
    //! \{
    int CoapPort;
    int IpcPort;
    bool Verbose;
    char * EndPointName;
    char * BootStrap;
    //! \}
} Options;

/***************************************************************************************************
 * Globals
 **************************************************************************************************/

//! \{

Options options =
{
    .CoapPort = COAP_PORT,
    .IpcPort = IPC_PORT,
    .Verbose = false,
    .BootStrap = "coap://["BOOTSTRAP_IPv6_ADDR"]:"BOOTSTRAP_PORT"/",
    .EndPointName = END_POINT_NAME,
};

/***************************************************************************************************
 * Implementation
 **************************************************************************************************/


void ConstructObjectTree(AwaStaticClient *client)
{
    AwaError err = DefineDeviceObject(client);
    StopIfErrorMsg(err, "Problems with Device Object.");

    err = TemperatureObject_DefineObjectsAndResources(client, -25, 85);
    StopIfErrorMsg(err, "Problems with Temperature object.");

    err = BarometerObject_DefineObjectsAndResources(client, 500, 1500);
    StopIfErrorMsg(err, "Problems with Barometer object.");

    err = HumidityObject_DefineObjectsAndResources(client, 0, 100);
    StopIfErrorMsg(err, "Problems with Temperature object.");

}

void AwaStaticClient_Start(AwaStaticClient *client)
{
    GPIO_SET(LED1_PORT, LED1_PIN);
    AwaStaticClient_SetLogLevel((options.Verbose) ? AwaLogLevel_Debug : AwaLogLevel_Warning);
    printf("LWM2M client - CoAP port %d\n", options.CoapPort);
    printf("LWM2M client - IPC port %d\n", options.IpcPort);

    AwaError err = AwaStaticClient_SetEndPointName(client, options.EndPointName);
    StopIfErrorMsg(err, "Can't set endpoint name.");

    err = AwaStaticClient_SetCoAPListenAddressPort(client, "0.0.0.0", options.CoapPort);
    StopIfErrorMsg(err, "Problems setting Coap listen port.");

    err = AwaStaticClient_SetBootstrapServerURI(client, options.BootStrap);
    StopIfErrorMsg(err, "Problems with bootstrap URI.");
    err = AwaStaticClient_Init(client);
    StopIfErrorMsg(err, "Problems while client init.");

    ConstructObjectTree(client);
}

void ConfigureLEDs()
{
    GPIO_CONFIGURE_AS_DIGITAL(LED1_PORT, LED1_PIN);
    GPIO_CONFIGURE_AS_OUTPUT(LED1_PORT, LED1_PIN);
    GPIO_CONFIGURE_AS_DIGITAL(LED2_PORT, LED2_PIN);
    GPIO_CONFIGURE_AS_OUTPUT(LED2_PORT, LED2_PIN);
    GPIO_CLR(LED2_PORT, LED2_PIN);
    GPIO_CLR(LED1_PORT, LED1_PIN);
}

PROCESS(lwm2m_client, "LwM2M Client");
AUTOSTART_PROCESSES(&lwm2m_client);

PROCESS_THREAD(lwm2m_client, ev, data)
{
    PROCESS_BEGIN();
    PROCESS_PAUSE();

    printf("Starting LWM2M Client for weather-station-weather-sensor\n");

#ifdef RF_CHANNEL
    printf("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
    printf("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

    printf("uIP buffer: %u\n", UIP_BUFSIZE);
    printf("LL header: %u\n", UIP_LLH_LEN);
    printf("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
#ifdef REST_MAX_CHUNK_SIZE
    printf("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);
#endif

    uip_ipaddr_t ipaddr;
    uip_ip6addr(&ipaddr, BOOTSTRAP_IPv6_ADDR1, BOOTSTRAP_IPv6_ADDR2, BOOTSTRAP_IPv6_ADDR3,
        BOOTSTRAP_IPv6_ADDR4, BOOTSTRAP_IPv6_ADDR5, BOOTSTRAP_IPv6_ADDR6, BOOTSTRAP_IPv6_ADDR7,
        BOOTSTRAP_IPv6_ADDR8);
    uip_ds6_defrt_add(&ipaddr, 0);

    static AwaStaticClient *client;
    client = AwaStaticClient_New();
    AwaStaticClient_Start(client);

    /* Define application-specific events here. */
    static double tempC = 0;
    static double humidity = 0;
    static double pressure = 0;
    
    i2c_init();

    if (weather_click_enable(0) < 0) {
	fprintf(stderr, "Failed to enable weather click");
    }

    if (weather_click_read_measurements(0, &tempC, &pressure, &humidity) < 0) {
	fprintf(stderr, "Failed to read mesaurement\n");
    }
   

    while(1)
    {
  
	static struct etimer et;
        static int WaitTime = 0;
        AwaStaticClient_Process(client);
      
	if (weather_click_read_measurements(0, &tempC, &pressure, &humidity) < 0) {
	    fprintf(stderr, "Failed to read mesaurement\n");
	}

        printf("Measured temp: %f, pressure:%f, humidity:%f\n", tempC, pressure, humidity);

	AwaError err = TemperatureObject_SetTemperature(client, tempC);
        InformIfErrorAndContinueMsg(err, "Error sending temperature\n");

        err = BarometerObject_SetPressure(client, pressure);
        InformIfErrorAndContinueMsg(err, "Error sending pressure\n");

	err = HumidityObject_SetHumidity(client, humidity);
        InformIfErrorAndContinueMsg(err, "Error sending humidity\n");

	printf("Waiting 60 seconds... \n");
	static int i = 0;
	for (i = 0; i < 30; i++) {
	    GPIO_SET(LED1_PORT, LED1_PIN);
            WaitTime = 1;
            etimer_set(&et, (WaitTime * CLOCK_SECOND));
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
            GPIO_CLR(LED1_PORT, LED1_PIN);
	    GPIO_SET(LED2_PORT, LED2_PIN);
	    WaitTime = 1;
 	    etimer_set(&et, (WaitTime * CLOCK_SECOND));
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
            GPIO_CLR(LED2_PORT, LED2_PIN);
	}
        
    }

    PROCESS_END();
}
