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

#include "sdcard.h"
#include "fat32.h"


SemaphoreHandle_t Semaforo_PWM;
SemaphoreHandle_t Semaforo_CAR;
SemaphoreHandle_t Semaforo_MET;
SemaphoreHandle_t Semaforo_NOR;

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
#define PORT(x) 	((uint8_t) x)
#define PIN(x)		((uint8_t) x)
#define TICKRATE_HZ1 (20000)	// 20000 ticks por segundo
#define TICKRATE_HZ2 (10000)	// 10000 tick por segundo



#define PUL_FOR		((uint8_t) 2)
#define PUL_BACK	((uint8_t) 2)
#define PUL_LEFT	((uint8_t) 0)
#define PUL_RIG		((uint8_t) 0)
#define PUL_STOP	((uint8_t) 0)
#define AL_CARB		((uint8_t) 2)
#define AL_MET		((uint8_t) 1)
#define PUL_CUAD1	((uint8_t) 0)
#define PUL_CUAD2	((uint8_t) 0)

#define LED_GASES	((uint8_t) 2)//LED Indicador de gases
#define MOTOR		((uint8_t) 0)//SALIDA DEL PWM
#define SAL_IZQ		((uint8_t) 0)
#define SAL_DER		((uint8_t) 0)
#define SAL_FOR		((uint8_t) 2)


#define PUL_FOR_PIN			((uint8_t) 4)
#define PUL_BACK_PIN		((uint8_t) 3)
#define PUL_LEFT_PIN		((uint8_t) 16)
#define PUL_RIG_PIN			((uint8_t) 15)
#define PUL_STOP_PIN		((uint8_t) 17)
#define AL_CARB_PIN			((uint8_t) 10)
#define AL_MET_PIN			((uint8_t) 31)
#define PUL_CUAD1_PIN		((uint8_t) 24)
#define PUL_CUAD2_PIN		((uint8_t) 25)

#define LED_GASES_PIN		((uint8_t) 1)
#define MOTOR_PIN			((uint8_t) 26)
#define SAL_IZQ_PIN		((uint8_t) 1)
#define SAL_DER_PIN		((uint8_t) 2)
#define SAL_FOR_PIN		((uint8_t) 0)

/*	Fin Definiciones pines	*/

#define OUTPUT		((uint8_t) 1)
#define INPUT		((uint8_t) 0)

#define ON			((uint8_t) 1)
#define OFF			((uint8_t) 0)

typedef struct
{
	uint32_t 	Ton;
	uint32_t 	Toff;
	SemaphoreHandle_t semaforo;
}LED_GPIO;

LED_GPIO LED_Struct;

void uC_StartUp (void)
{
	Chip_GPIO_Init (LPC_GPIO);

	Chip_IOCON_PinMux (LPC_IOCON , PUL_FOR , PUL_FOR_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_BACK , PUL_BACK_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_LEFT , PUL_LEFT_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_RIG , PUL_RIG_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_STOP , PUL_STOP_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , AL_CARB , AL_CARB_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , AL_MET , AL_MET_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_CUAD1 , PUL_CUAD1_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , PUL_CUAD2 , PUL_CUAD2_PIN, IOCON_MODE_INACT , IOCON_FUNC0);

	Chip_IOCON_PinMux (LPC_IOCON , LED_GASES , LED_GASES_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , MOTOR , MOTOR_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , SAL_IZQ , SAL_IZQ_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , SAL_DER , SAL_DER_PIN, IOCON_MODE_INACT , IOCON_FUNC0);
	Chip_IOCON_PinMux (LPC_IOCON , SAL_FOR , SAL_FOR_PIN, IOCON_MODE_INACT , IOCON_FUNC0);

	Chip_GPIO_SetDir (LPC_GPIO , PUL_FOR , PUL_FOR_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_BACK , PUL_BACK_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_LEFT , PUL_LEFT_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_RIG , PUL_RIG_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_STOP , PUL_STOP_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , AL_CARB , AL_CARB_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , AL_MET , AL_MET_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_CUAD1 , PUL_CUAD1_PIN , INPUT);
	Chip_GPIO_SetDir (LPC_GPIO , PUL_CUAD2 , PUL_CUAD2_PIN , INPUT);

	Chip_GPIO_SetDir (LPC_GPIO , LED_GASES , LED_GASES_PIN , OUTPUT);
	Chip_GPIO_SetDir (LPC_GPIO , MOTOR , MOTOR_PIN , OUTPUT);
	Chip_GPIO_SetDir (LPC_GPIO , SAL_IZQ , SAL_IZQ_PIN , OUTPUT);
	Chip_GPIO_SetDir (LPC_GPIO , SAL_DER , SAL_DER_PIN , OUTPUT);
	Chip_GPIO_SetDir (LPC_GPIO , SAL_FOR , SAL_FOR_PIN , OUTPUT);

}


/* ENTRADAS */
static void vTaskPulsadores(void *pvParameters)
{
	uint8_t envio=50;
	uint8_t estadoPWM = 0; //Estado del PWM
	uint8_t estadoEnc = 0; //Estado del encoder escrito en binario
	uint8_t estadoEncPrevio = 0; //Estado previo del encoder escrito en binario
	uint32_t TestigoStop = FALSE;
	uint32_t TestigoLeft = FALSE;
	uint32_t TestigoRight = FALSE;
	uint32_t TestigoFor = FALSE;
	uint32_t TestigoBack = FALSE;


	//Leo el encoder 1 vez para saber donde inicia
	estadoEnc = 2*Chip_GPIO_GetPinState(LPC_GPIO, PUL_CUAD1, PUL_CUAD2_PIN);//suma 2 si esta en 1
	if(!Chip_GPIO_GetPinState(LPC_GPIO, PUL_CUAD2, PUL_CUAD2_PIN))
	estadoEnc = estadoEnc + 1; //suma 1 si esta en 0 caso contrario no suma nada
	estadoEncPrevio = estadoEnc;
	//Estados del encoder 00-> 0 01-> 1 11-> 2 10-> 3

	while (1)
	{
		vTaskDelay( 50 / portTICK_PERIOD_MS );//Muestreo cada 50 mseg

		//Control de stop
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_STOP, PUL_STOP_PIN) && TestigoStop == FALSE)
		{
			if(!estadoPWM)//si esta apagado
			{
				Chip_TIMER_Enable (LPC_TIMER0); //Habilito el timer
				estadoPWM = 1;
				TestigoStop = TRUE;
			}
			if(estadoPWM)//si esta prendido
			{
				Chip_TIMER_Disable (LPC_TIMER0); //Deshabilito el timer
				Chip_GPIO_SetPinOutLow (LPC_GPIO , MOTOR , MOTOR_PIN); //Apago la salida
				estadoPWM = 0;
				TestigoStop = TRUE;
			}
		}
		if (Chip_GPIO_GetPinState (LPC_GPIO , PUL_STOP , PUL_STOP_PIN) == FALSE)
			TestigoStop = FALSE;

		//Control izquierda
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_LEFT, PUL_LEFT_PIN) && TestigoLeft == FALSE)
		{
			Chip_GPIO_SetPinOutLow (LPC_GPIO , SAL_DER , SAL_DER_PIN); //Apago la salida derecha
			Chip_GPIO_SetPinOutHigh (LPC_GPIO , SAL_IZQ , SAL_IZQ_PIN); //Prendo la salida izquierda
			TestigoLeft = TRUE;
		}
		if (Chip_GPIO_GetPinState (LPC_GPIO , PUL_LEFT , PUL_LEFT_PIN) == FALSE)
			TestigoLeft = FALSE;

		//Control derecha
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_RIG, PUL_RIG_PIN) && TestigoRight == FALSE)
		{
			Chip_GPIO_SetPinOutLow (LPC_GPIO , SAL_IZQ , SAL_IZQ_PIN); //Apago la salida izquierda
			Chip_GPIO_SetPinOutHigh (LPC_GPIO , SAL_DER , SAL_DER_PIN); //Prendo la salida derecha
			TestigoRight = TRUE;
		}
		if (Chip_GPIO_GetPinState (LPC_GPIO , PUL_RIG , PUL_RIG_PIN) == FALSE)
			TestigoRight = FALSE;

		//Control adelante
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_FOR, PUL_FOR_PIN) && TestigoFor == FALSE)
		{
			Chip_GPIO_SetPinOutHigh (LPC_GPIO , SAL_FOR , SAL_FOR_PIN); //Prendo la salida derecha
			TestigoFor = TRUE;
		}
		if (Chip_GPIO_GetPinState (LPC_GPIO , PUL_FOR , PUL_FOR_PIN) == FALSE)
			TestigoFor = FALSE;

		//Control reversa
		if(Chip_GPIO_GetPinState(LPC_GPIO, PUL_BACK, PUL_BACK_PIN) && TestigoBack == FALSE)
		{
			Chip_GPIO_SetPinOutLow (LPC_GPIO , SAL_FOR , SAL_FOR_PIN); //Prendo la salida derecha
			TestigoBack = TRUE;
		}
		if (Chip_GPIO_GetPinState (LPC_GPIO , PUL_BACK , PUL_BACK_PIN) == FALSE)
			TestigoBack = FALSE;

		//Encoder
		estadoEnc = 2*Chip_GPIO_GetPinState(LPC_GPIO, PUL_CUAD1, PUL_CUAD2_PIN);//suma 2 si esta en 1
		if(!Chip_GPIO_GetPinState(LPC_GPIO, PUL_CUAD2, PUL_CUAD2_PIN))
		estadoEnc = estadoEnc + 1; //suma 1 si esta en 0 caso contrario no suma nada
		//Estados del encoder 00-> 0 01-> 1 11-> 2 10-> 3

		if (estadoEnc != estadoEncPrevio) // hubo una variación en el encoder
		{
			if(estadoEnc < estadoEncPrevio || (estadoEnc == 3 && estadoEncPrevio == 0) )
			{
				if(envio < 90)
				envio = envio + 10; //envio contiene el porcentaje del PWM

				estadoEncPrevio = estadoEnc;
			}

			if(estadoEnc > estadoEncPrevio || (estadoEnc == 0 && estadoEncPrevio == 3) )
			{
				if(envio > 10)
				envio = envio - 10; //envio contiene el porcentaje del PWM

				estadoEncPrevio = estadoEnc;
			}

			//Como quiero el x% de duty sera 1/x la F a trabajar yel dividido 100 es por el porcentaje
			Chip_TIMER_SetMatch(LPC_TIMER0, 0, ((SystemCoreClock / 4) /(TICKRATE_HZ2 * (1/envio/100) )));
		}


		//Analizo pulsadores de alarma
		if(Chip_GPIO_GetPinState(LPC_GPIO, AL_CARB, AL_CARB_PIN))
		{
			xSemaphoreGive(Semaforo_CAR);//Doy el semaforo en caso de que el led tenga que encenderse por carbono
		}

		else if (Chip_GPIO_GetPinState(LPC_GPIO, AL_MET, AL_MET_PIN))
		{
			xSemaphoreGive(Semaforo_MET);//Doy el semaforo en caso de que el led tenga que encenderse por carbono
		}
		else//Significa que no hay ninguna alarma
		{
			xSemaphoreGive(Semaforo_NOR);//Doy el semaforo en caso de que el led tenga que encenderse por carbono
		}


	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* xTaskLed1:
* Tarea que se encarga de controlar el tiempo encendido o apagada la salida
*/

//INCOMPLETA faltaria tarea que instancia.
static void xTaskLed(void *pvParameters)
{

	LED_GPIO * Buffer;

	Buffer = (LED_GPIO*) pvParameters;


	while (1)
	{
		xSemaphoreTake(Buffer->semaforo , portMAX_DELAY );
		Chip_GPIO_SetPinOutLow (LPC_GPIO , LED_GASES , LED_GASES_PIN); //Apago la salida del led
		vTaskDelay( Buffer->Ton / portTICK_PERIOD_MS );
		xSemaphoreTake(Buffer->semaforo , portMAX_DELAY );
		Chip_GPIO_SetPinOutHigh (LPC_GPIO , LED_GASES , LED_GASES_PIN); //Prendo la salida del led
		vTaskDelay( Buffer->Toff / portTICK_PERIOD_MS );
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

		/* Enable timer 1 clock */
		Chip_TIMER_Init(LPC_TIMER0);	//Enciende el modulo

		Chip_TIMER_Reset(LPC_TIMER0);									//Borra la cuenta

		//MATCH 0: NO RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);						//Habilita interrupcion del match 0 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 0, ((SystemCoreClock / 4) / TICKRATE_HZ1));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 500ms)
		Chip_TIMER_ResetOnMatchDisable(LPC_TIMER0, 0);					//Cada vez que llega al match NO resetea la cuenta

		//MATCH 1: SI RESETEA LA CUENTA
		Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);						//Habilita interrupcion del match 1 timer 0
		Chip_TIMER_SetMatch(LPC_TIMER0, 1, ((SystemCoreClock / 4) / TICKRATE_HZ2));	//Le asigna un valor al match - seteo la frec a la que quiero que el timer me interrumpa (Ej 1s)
		Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);					//Cada vez que llega al match resetea la cuenta

		//Chip_TIMER_Enable(LPC_TIMER0);									//Comienza a contar (hago que se habilite al encender)
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Programa principal:


int main(void)
{
	uC_StartUp ();
	SystemCoreClockUpdate();


	vSemaphoreCreateBinary(Semaforo_PWM);
	vSemaphoreCreateBinary(Semaforo_CAR);
	vSemaphoreCreateBinary(Semaforo_MET);
	vSemaphoreCreateBinary(Semaforo_NOR);


	xSemaphoreTake(Semaforo_CAR, portMAX_DELAY); //semaforo de carbono
	xSemaphoreTake(Semaforo_MET, portMAX_DELAY); //semaforo de metano

	/*
	 * TAREA CON MAYOR PRIORIDAD (+2UL) QUE INICIALIZA EL TIMER Y LUEGO SE AUTOELIMINA
	 * */
	xTaskCreate(vTaskInicTimer, (char *) "vTaskInicTimer",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3UL),
				(xTaskHandle *) NULL);

	xTaskCreate(vTaskPulsadores, (char *) "vTaskPulsadores",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);


	LED_Struct.semaforo = Semaforo_NOR;
	LED_Struct.Ton = 250;
	LED_Struct.Toff = 9750;

	xTaskCreate(xTaskLed, (char *) "xTaskLedNor",
				configMINIMAL_STACK_SIZE, &LED_Struct , (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	LED_Struct.semaforo = Semaforo_CAR;
	LED_Struct.Ton = 500;
	LED_Struct.Toff = 500;

	xTaskCreate(xTaskLed, (char *) "xTaskLedCAR",
				configMINIMAL_STACK_SIZE, &LED_Struct, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	LED_Struct.semaforo = Semaforo_MET;
	LED_Struct.Ton = 125;
	LED_Struct.Toff = 125;

	xTaskCreate(xTaskLed, (char *) "xTaskLedMET",
				configMINIMAL_STACK_SIZE, &LED_Struct, (tskIDLE_PRIORITY + 1UL),
				(xTaskHandle *) NULL);

	xTaskCreate(xTaskPWM, (char *) "xTaskPWM",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2UL),
				(xTaskHandle *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

