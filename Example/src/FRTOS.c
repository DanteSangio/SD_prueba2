/*
===============================================================================
 Name        : FRTOS.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <cr_section_macros.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SemaphoreHandle_t Semaforo_On;
SemaphoreHandle_t Semaforo_PWM;
QueueHandle_t Cola_PWM;
QueueHandle_t Cola_LED;

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
#define PORT(x) 	((uint8_t) x)
#define PIN(x)		((uint8_t) x)
#define TICKRATE_HZ1 (4*2)	// 4: cte de frec - 2 ticks por segundo
#define TICKRATE_HZ2 (4*20000)	// 4: cte de frec -  tick por segundo


/*	Definiciones pines joystick	*/
#define PUL_ON		((uint8_t) 0)//centro del joystick
#define PUL_90		((uint8_t) 2)
#define PUL_70		((uint8_t) 0)
#define PUL_50		((uint8_t) 0)
#define PUL_EMER	((uint8_t) 0)
#define FAULT		((uint8_t) 1)//wakeup de base board pin que recibe si el motor esta tomando sobre corriente
#define LED_EMER	((uint8_t) 2)//LED EMERGENCIA
#define MOTOR	((uint8_t) 0)//SALIDA DEL MOTOR ( NO ES UNA CONEXION QUE FUNCIONE)


#define PUL_ON_PIN			((uint8_t) 17)
#define PUL_90_PIN			((uint8_t) 3)
#define PUL_70_PIN			((uint8_t) 16)
#define PUL_50_PIN			((uint8_t) 15)
#define PUL_EMER_PIN		((uint8_t) 15)
#define FAULT_PIN			((uint8_t) 31)
#define LED_EMER_PIN		((uint8_t) 0)
#define MOTOR_PIN			((uint8_t) 22)

/*	Fin Definiciones pines	*/

#define OUTPUT		((uint8_t) 1)
#define INPUT		((uint8_t) 0)

#define ON			((uint8_t) 1)
#define OFF			((uint8_t) 0)

void uC_StartUp (void)
{
	Chip_GPIO_Init (LPC_GPIO);

	Chip_IOCON_PinMux (LPC_IOCON , PUL_ON , PUL_ON_PIN, IOCON_MODE_PULLDOWN , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_90 , PUL_90_PIN, IOCON_MODE_PULLDOWN , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_70 , PUL_70_PIN, IOCON_MODE_PULLDOWN , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_50 , PUL_50_PIN, IOCON_MODE_PULLDOWN , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_EMER , PUL_EMER_PIN, IOCON_MODE_PULLDOWN , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , FAULT , FAULT_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , LED_EMER , LED_EMER_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , MOTOR , MOTOR_PIN, IOCON_MODE_INACT , IOCON_FUNC0);


	Chip_GPIO_SetDir (LPC_GPIO , PUL_ON , PUL_ON_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_90 , PUL_90_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_70 , PUL_70_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_50 , PUL_50_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_EMER , PUL_EMER_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , FAULT , FAULT_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , LED_EMER , LED_EMER_PIN , OUTPUT);
	Chip_GPIO_SetDir (LPC_GPIO , MOTOR , MOTOR_PIN , OUTPUT);



}


/* ENTRADAS CON MENOR PRIORIDAD */
static void vTaskPulsadores(void *pvParameters)
{
	uint8_t envio=0;
	while (1)
	{
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_ON,PUL_ON_PIN))
		{
			xSemaphoreGive(Semaforo_On );
			vTaskDelay( 3000 / portTICK_PERIOD_MS );//Delay de 3 seg hasta que suelte el pulsador
		}

		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_90,PUL_90_PIN))
		{
			envio = 90; //Porcentaje del PWM
			xQueueSendToBack(Cola_PWM, &envio, portMAX_DELAY);
			vTaskDelay( 3000 / portTICK_PERIOD_MS );//Delay de 3 seg hasta que suelte el pulsador
		}

		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_70, PUL_70_PIN))
		{
			envio = 70; //Porcentaje del PWM
			xQueueSendToBack(Cola_PWM, &envio, portMAX_DELAY);
			vTaskDelay( 3000 / portTICK_PERIOD_MS );//Delay de 3 seg hasta que suelte el pulsador
		}

		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_50, PUL_50_PIN))
		{
			envio = 50; //Porcentaje del PWM
			xQueueSendToBack(Cola_PWM, &envio, portMAX_DELAY);
			vTaskDelay( 3000 / portTICK_PERIOD_MS );//Delay de 3 seg hasta que suelte el pulsador
		}
	}
}

/* ENTRADAS CON MAYOR PRIORIDAD */
static void vTaskEmergencia(void *pvParameters)
{
	uint8_t envio=0;
	while (1)
	{
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_EMER, PUL_EMER_PIN))
		{
			xSemaphoreTake(Semaforo_On , portMAX_DELAY );// en caso de parada de emergencia tomo el semaforo de funcionamiento
			envio = 20; //Frecuencia a la que tiene que parpadear el led*10 (así utilizo enteros)
			xQueueSendToBack(Cola_LED, &envio, portMAX_DELAY);//Lo envio a la cola que maneja la F del led
			vTaskDelay(1000/portTICK_RATE_MS); //Delay de 1 seg para que se ejecuten las otras acciones
		}
		if(Chip_GPIO_GetPinState(LPC_GPIO, FAULT, FAULT_PIN))
		{
			xSemaphoreTake(Semaforo_On , portMAX_DELAY );// en caso de parada de emergencia tomo el semaforo de funcionamiento
			envio = 5; //Frecuencia a la que tiene que parpadear el led*10 (así utilizo enteros)
			xQueueSendToBack(Cola_LED, &envio, portMAX_DELAY);//Lo envio a la cola que maneja la F del led
			vTaskDelay(1000/portTICK_RATE_MS); //Delay de 1 seg para que se ejecuten las otras acciones
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskMatch0:
* Tarea que se encarga de controlar el % de tiempo encendido o apagada la salida
*/
static void xTaskMatch0(void *pvParameters)
{
	uint8_t Ticks_HZ_Match0 = 0;//el PWM inicializa en 0
	uint8_t Receive=0;
	uint32_t timerFreq;

	while (1)
	{
		xQueueReceive(Cola_PWM,&Receive,portMAX_DELAY);	//Para recibir de la Cola_1

		if(Receive)//Receive esta en porcentaje
		{
			timerFreq = Chip_Clock_GetSystemClockRate();	//Obtiene la frecuencia a la que esta corriendo el uC
			//Como quiero el x% de duty sera 1/x la F a trabajar yel dividido 100 es por el porcentaje
			Chip_TIMER_SetMatch(LPC_TIMER0, 0, (timerFreq /(TICKRATE_HZ2 * (1/Receive/100) )));
			Receive=0;										//Reestablece la variable
		}
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskLed:
* Tarea que se encarga de controlar el tiempo encendido o apagada la salida
*/
static void xTaskLed(void *pvParameters)
{
	uint8_t Receive=0;

	xQueueReceive(Cola_LED,&Receive,portMAX_DELAY);	//Para recibir de la Cola_Led la f*10 de trabajo

	while (1)
	{
		Chip_GPIO_SetPinToggle(LPC_GPIO,LED_EMER,LED_EMER_PIN);
		vTaskDelay((1/Receive/10)/portTICK_RATE_MS); //Parpadeo del led a una f receive/10
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskPWM:
* Tarea que se encarga de controlar el tiempo encendido o apagada la salida
*/
static void xTaskPWM(void *pvParameters)
{

	while (1)
	{
		xSemaphoreTake(Semaforo_On , portMAX_DELAY );//Intento tomar semaforo de encendido, caso de no poder espero hasta que lo este
		xSemaphoreGive(Semaforo_On);//Entrego el semaforo para el caso que se apriete el boton de apagado

		xSemaphoreTake(Semaforo_PWM, portMAX_DELAY);//Tomo el semaforo del PWM - debe liberarlo la interrupcion
		Chip_GPIO_SetPinToggle(LPC_GPIO,MOTOR, MOTOR_PIN);
	}
	vTaskDelete(NULL);	//Borra la tarea si sale del while 1
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* vTaskInicTimer:
 * Tarea que se encarga de inicializar el TIMER0 y luego se autoelimina
*/
static void vTaskInicTimer(void *pvParameters)
{
	while (1)
	{
		uint32_t timerFreq;

		/* Enable timer 1 clock */
		Chip_TIMER_Init(LPC_TIMER0);	//Enciende el modulo
		/* Timer rate is system clock rate */
		timerFreq = Chip_Clock_GetSystemClockRate();					//Obtiene la frecuencia a la que esta corriendo el uC
		/* Timer setup for match and interrupt at TICKRATE_HZ */
		Chip_TIMER_Reset(LPC_TIMER0);									//Borra la cuenta

		//MATCH 0: NO RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);						//Habilita interrupcion del match 0 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 0, (timerFreq / TICKRATE_HZ1));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 500ms)
		Chip_TIMER_ResetOnMatchDisable(LPC_TIMER0, 0);					//Cada vez que llega al match NO resetea la cuenta

		//MATCH 1: SI RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);						//Habilita interrupcion del match 1 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / TICKRATE_HZ2));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 1s)
		Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);					//Cada vez que llega al match resetea la cuenta

		Chip_TIMER_Enable(LPC_TIMER0);									//Comienza a contar
		/* Enable timer interrupt */ 		//El NVIC asigna prioridades de las interrupciones (prioridad de 0 a inf)
		NVIC_ClearPendingIRQ(TIMER0_IRQn);
		NVIC_EnableIRQ(TIMER0_IRQn);		//Enciende la interrupcion que acabamos de configurar

		vTaskDelete(NULL);	//Borra la tarea, no necesitaria el while(1)
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* TIMER0_IRQHandler:
 * Controlador del TIMER0
*/
void TIMER0_IRQHandler(void)
{
	BaseType_t Testigo=pdFALSE;

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 0))
	{
		Chip_TIMER_ClearMatch(LPC_TIMER0, 0);				//Resetea match

		xSemaphoreGiveFromISR(Semaforo_PWM, &Testigo);		//Devuelve si una de las tareas bloqueadas tiene mayor prioridad que la actual

		portYIELD_FROM_ISR(Testigo);						//Si testigo es TRUE -> ejecuta el scheduler
	}

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 1))
	{
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);				//Resetea match

		xSemaphoreGiveFromISR(Semaforo_PWM, &Testigo);		//Devuelve si una de las tareas bloqueadas tiene mayor prioridad que la actual

		portYIELD_FROM_ISR(Testigo);						//Si testigo es TRUE -> ejecuta el scheduler
	}
}



int main(void)
{
	uC_StartUp ();
	SystemCoreClockUpdate();

	vSemaphoreCreateBinary(Semaforo_On);
	vSemaphoreCreateBinary(Semaforo_PWM);

	Cola_PWM = xQueueCreate(1, sizeof(uint8_t));	//Creación de una cola de tamaño 1 y tipo uint8
	Cola_PWM = xQueueCreate(1, sizeof(uint8_t));	//Creación de una cola de tamaño 1 y tipo uint8


	xSemaphoreTake(Semaforo_On , portMAX_DELAY );//semaforo de encendido


	/*
	 * TAREA CON MAYOR PRIORIDAD (+2UL) QUE INICIALIZA EL TIMER Y LUEGO SE AUTOELIMINA
	 * */
	xTaskCreate(vTaskInicTimer, (char *) "vTaskInicTimer",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);

	xTaskCreate(vTaskPulsadores, (char *) "vTaskPulsadores",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	xTaskCreate(vTaskEmergencia, (char *) "vTaskEmergencia",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);

	xTaskCreate(xTaskMatch0, (char *) "xTaskMatch0",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	xTaskCreate(xTaskLed, (char *) "xTaskLed",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	xTaskCreate(xTaskPWM, (char *) "xTaskPWM",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

