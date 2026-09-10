/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Laboratorio 6 - Post Lab
  *                   Control 1: 2 potenciometros con ADC + DMA
  *                   Control 2: ATmega328P recibido por UART con interrupciones
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/*
 * El ADC es de 12 bits.
 *
 * Valor minimo = 0
 * Valor maximo = 4095
 *
 * La mitad del ADC es aproximadamente 2048.
 */
#define MITAD_ADC 2048

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/*
 * VARIABLES DEL CONTROL 1
 *
 * DMA guardara aqui las lecturas de los dos potenciometros.
 *
 * ADCValor[0] = Potenciometro 1 = PA0 = ADC1_IN0
 * ADCValor[1] = Potenciometro 2 = PA1 = ADC1_IN1
 */
uint16_t ADCValor[2];


/*
 * Bandera que nos indica que DMA termino
 * de leer los dos canales del ADC.
 *
 * 0 = no hay lectura nueva
 * 1 = lectura nueva disponible
 */
volatile uint8_t estadoADC = 0;


/*
 * Guardamos el estado anterior de cada potenciometro.
 *
 * Esto nos permite enviar un mensaje solamente cuando
 * el potenciometro cambia de direccion.
 *
 * Usamos 2 como estado inicial porque los estados
 * normales solamente seran 0 o 1.
 */
uint8_t estadoPot1Anterior = 2;
uint8_t estadoPot2Anterior = 2;


/*
 * VARIABLES DEL CONTROL 2
 *
 * rxATmega es donde USART1 guardara el byte
 * recibido desde el ATmega328P.
 *
 * El ATmega envia:
 *
 * U = Arriba
 * D = Abajo
 * R = Derecha
 * L = Izquierda
 * A = Accion A
 * B = Accion B
 */
uint8_t rxATmega = 0;


/*
 * Aqui guardamos una copia del ultimo comando
 * recibido desde el ATmega.
 */
volatile uint8_t comandoATmega = 0;


/*
 * Bandera que nos indica que USART1 recibio
 * un nuevo comando.
 *
 * 0 = no hay comando nuevo
 * 1 = hay comando nuevo
 */
volatile uint8_t estadoUART = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */


  /* MCU Configuration--------------------------------------------------------*/

  /*
   * Inicializamos HAL y reiniciamos los perifericos.
   */
  HAL_Init();


  /* USER CODE BEGIN Init */

  /* USER CODE END Init */


  /*
   * Configuramos el reloj del microcontrolador.
   */
  SystemClock_Config();


  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */


  /*
   * Inicializamos todos los perifericos que
   * configuramos desde CubeMX.
   */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();


  /* USER CODE BEGIN 2 */

  /*
   * CONTROL 1
   *
   * Iniciamos ADC1 utilizando DMA.
   *
   * DMA realizara dos transferencias:
   *
   * ADCValor[0] = Potenciometro 1
   * ADCValor[1] = Potenciometro 2
   */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValor, 2);


  /*
   * CONTROL 2
   *
   * Dejamos USART1 esperando UN byte enviado
   * por el ATmega328P.
   *
   * La terminacion "_IT" significa que la recepcion
   * se realiza utilizando INTERRUPCIONES.
   *
   *
   * Cuando llegue un byte se ejecutara automaticamente:
   *
   * HAL_UART_RxCpltCallback()
   */
  HAL_UART_Receive_IT(&huart1, &rxATmega, 1);

  /* USER CODE END 2 */


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


    /*
     * CONTROL 1
     * DOS POTENCIOMETROS CON ADC + DMA
     *
     * El callback del ADC coloca estadoADC = 1 cuando
     * DMA termina de obtener las dos lecturas.
     */
    if (estadoADC)
    {
      /*
       * Limpiamos la bandera porque ya vamos
       * a procesar la lectura.
       */
      estadoADC = 0;


      /*
       * Variables para guardar el estado actual
       * de cada potenciometro.
       *
       * 0 = mitad inferior
       * 1 = mitad superior
       */
      uint8_t estadoPot1Actual;
      uint8_t estadoPot2Actual;


      /*
       * POTENCIOMETRO 1
       *
       * Si ADCValor[0] esta arriba de la mitad:
       *
       * Arriba
       *
       * Si esta abajo de la mitad:
       *
       * Abajo
       */
      if (ADCValor[0] >= MITAD_ADC)
      {
        estadoPot1Actual = 1;
      }
      else
      {
        estadoPot1Actual = 0;
      }


      /*
       * Solo mandamos un mensaje si la direccion cambio.
       *
       * Esto evita llenar Hercules con el mismo mensaje
       * miles de veces.
       */
      if (estadoPot1Actual != estadoPot1Anterior)
      {
        /*
         * Guardamos el nuevo estado.
         */
        estadoPot1Anterior = estadoPot1Actual;


        /*
         * Revisamos cual direccion corresponde.
         */
        if (estadoPot1Actual == 1)
        {
          /*
           * Potenciometro 1 arriba de la mitad.
           */
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 1: Arriba\r\n",
                            sizeof("Control 1: Arriba\r\n") - 1,
                            HAL_MAX_DELAY);
        }
        else
        {
          /*
           * Potenciometro 1 abajo de la mitad.
           */
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 1: Abajo\r\n",
                            sizeof("Control 1: Abajo\r\n") - 1,
                            HAL_MAX_DELAY);
        }
      }


      /*
       * POTENCIOMETRO 2
       *
       * Si ADCValor[1] esta arriba de la mitad:
       *
       * Izquierda
       *
       * Si esta abajo de la mitad:
       *
       * Derecha
       */
      if (ADCValor[1] >= MITAD_ADC)
      {
        estadoPot2Actual = 1;
      }
      else
      {
        estadoPot2Actual = 0;
      }


      /*
       * Nuevamente solamente enviamos informacion
       * cuando cambia la direccion.
       */
      if (estadoPot2Actual != estadoPot2Anterior)
      {
        /*
         * Guardamos el nuevo estado.
         */
        estadoPot2Anterior = estadoPot2Actual;


        /*
         * Revisamos cual direccion corresponde.
         */
        if (estadoPot2Actual == 1)
        {
          /*
           * Potenciometro 2 arriba de la mitad.
           */
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 1: Izquierda\r\n",
                            sizeof("Control 1: Izquierda\r\n") - 1,
                            HAL_MAX_DELAY);
        }
        else
        {
          /*
           * Potenciometro 2 abajo de la mitad.
           */
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 1: Derecha\r\n",
                            sizeof("Control 1: Derecha\r\n") - 1,
                            HAL_MAX_DELAY);
        }
      }


      /*
       * Nuestro DMA esta configurado en modo NORMAL.
       *
       * Por eso, despues de terminar las dos conversiones,
       * tenemos que volver a iniciar ADC + DMA manualmente.
       *
       * Esto mantiene la misma estructura utilizada
       * en los ejemplos del profesor.
       */
      HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValor, 2);
    }



    /*
     * CONTROL 2
     * ATmega328P POR USART1
     *
     * Cuando USART1 recibe un byte por interrupcion,
     * el callback coloca estadoUART = 1.
     */
    if (estadoUART)
    {
      /*
       * Limpiamos la bandera porque ya vamos
       * a procesar el comando.
       */
      estadoUART = 0;


      /*
       * Revisamos cual caracter envio el ATmega.
       */
      switch (comandoATmega)
      {

        /*
         * U = Arriba
         */
        case 'U':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: Arriba\r\n",
                            sizeof("Control 2: Arriba\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * D = Abajo
         */
        case 'D':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: Abajo\r\n",
                            sizeof("Control 2: Abajo\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * R = Derecha
         */
        case 'R':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: Derecha\r\n",
                            sizeof("Control 2: Derecha\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * L = Izquierda
         */
        case 'L':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: Izquierda\r\n",
                            sizeof("Control 2: Izquierda\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * A = Accion A
         */
        case 'A':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: A\r\n",
                            sizeof("Control 2: A\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * B = Accion B
         */
        case 'B':

          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"Control 2: B\r\n",
                            sizeof("Control 2: B\r\n") - 1,
                            HAL_MAX_DELAY);

          break;


        /*
         * Si por alguna razon llega otro caracter,
         * simplemente lo ignoramos.
         */
        default:

          break;
      }
    }

  }

  /* USER CODE END 3 */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  /*
   * Habilitamos el reloj del modulo de potencia.
   */
  __HAL_RCC_PWR_CLK_ENABLE();


  /*
   * Configuracion del regulador interno.
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);


  /*
   * Configuramos el oscilador HSI y el PLL.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;


  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }


  /*
   * Configuramos los relojes principales.
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;

  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;


  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                          FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */


  ADC_ChannelConfTypeDef sConfig = {0};


  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */


  /*
   * CONFIGURACION GENERAL DEL ADC1
   */

  hadc1.Instance = ADC1;


  /*
   * Reloj del ADC:
   *
   * PCLK2 dividido entre 8.
   */
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;


  /*
   * Resolucion de 12 bits.
   *
   * El ADC entregara valores entre 0 y 4095.
   */
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;


  /*
   * Activamos Scan porque estamos utilizando
   * dos canales diferentes.
   */
  hadc1.Init.ScanConvMode = ENABLE;


  /*
   * No utilizamos conversion continua.
   *
   * Nosotros volveremos a iniciar DMA despues
   * de cada secuencia.
   */
  hadc1.Init.ContinuousConvMode = DISABLE;


  /*
   * No utilizamos modo discontinuo.
   */
  hadc1.Init.DiscontinuousConvMode = DISABLE;


  /*
   * La conversion sera iniciada por software.
   */
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;


  /*
   * Resultado alineado hacia la derecha.
   */
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;


  /*
   * Tenemos dos conversiones:
   *
   * Rank 1 = Canal 0
   * Rank 2 = Canal 1
   */
  hadc1.Init.NbrOfConversion = 2;


  /*
   * DMA trabaja en modo normal.
   */
  hadc1.Init.DMAContinuousRequests = DISABLE;


  /*
   * La bandera de fin de conversion aparece
   * despues de terminar toda la secuencia.
   */
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;


  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }


  /*
   * =========================================================
   * RANK 1
   * POTENCIOMETRO 1
   * PA0 = ADC1_IN0
   * =========================================================
   */

  sConfig.Channel = ADC_CHANNEL_0;

  sConfig.Rank = 1;

  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;


  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }


  /*
   * RANK 2
   * POTENCIOMETRO 2
   * PA1 = ADC1_IN1
   */

  sConfig.Channel = ADC_CHANNEL_1;

  sConfig.Rank = 2;

  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;


  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}


/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */


  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */


  /*
   * USART1
   *
   * USART1 se utiliza para recibir los comandos
   * enviados por el ATmega328P.
   *
   * PA10 = USART1_RX
   *
   * Configuracion:
   *
   * 9600 baudios
   * 8 bits
   * Sin paridad
   * 1 bit de parada
   */

  huart1.Instance = USART1;

  huart1.Init.BaudRate = 9600;

  huart1.Init.WordLength = UART_WORDLENGTH_8B;

  huart1.Init.StopBits = UART_STOPBITS_1;

  huart1.Init.Parity = UART_PARITY_NONE;

  huart1.Init.Mode = UART_MODE_TX_RX;

  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  huart1.Init.OverSampling = UART_OVERSAMPLING_16;


  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */


  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */


  /*
   * USART2
   *
   * USART2 se utiliza para mandar los resultados
   * hacia Hercules en la computadora.
   *
   * Configuracion:
   *
   * 9600 baudios
   * 8 bits
   * Sin paridad
   * 1 bit de parada
   */

  huart2.Instance = USART2;

  huart2.Init.BaudRate = 9600;

  huart2.Init.WordLength = UART_WORDLENGTH_8B;

  huart2.Init.StopBits = UART_STOPBITS_1;

  huart2.Init.Parity = UART_PARITY_NONE;

  huart2.Init.Mode = UART_MODE_TX_RX;

  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  huart2.Init.OverSampling = UART_OVERSAMPLING_16;


  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
}


/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /*
   * Habilitamos el reloj del DMA2.
   */
  __HAL_RCC_DMA2_CLK_ENABLE();


  /*
   * Habilitamos la interrupcion de DMA2 Stream 0.
   *
   * Cuando termina la transferencia de las dos
   * lecturas ADC se genera una interrupcion.
   */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);

  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};


  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */


  /*
   * Habilitamos los relojes de los puertos GPIO.
   */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  __HAL_RCC_GPIOH_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();


  /*
   */
  HAL_GPIO_WritePin(LD2_GPIO_Port,
                    LD2_Pin,
                    GPIO_PIN_RESET);


  /*
   */
  GPIO_InitStruct.Pin = B1_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);


  /*
   */
  GPIO_InitStruct.Pin = LD2_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);


  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}


/* USER CODE BEGIN 4 */


/*
 * CALLBACK DEL ADC + DMA
 *
 * Esta funcion se ejecuta automaticamente cuando
 * DMA termina de guardar las dos conversiones del ADC.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  /*
   * Verificamos que el evento corresponda al ADC1.
   */
  if (hadc->Instance == ADC1)
  {
    /*
     * Avisamos al while que ya tenemos
     * una lectura nueva.
     */
    estadoADC = 1;
  }
}



/*
 * CALLBACK DE USART1
 *
 * Esta funcion se ejecuta automaticamente cuando
 * USART1 termina de recibir el byte enviado
 * por el ATmega328P.
 *
 * Esta es la parte que nos permite cumplir
 * con la recepcion UART POR INTERRUPCIONES.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /*
   * Verificamos que el byte haya llegado
   * especificamente por USART1.
   */
  if (huart->Instance == USART1)
  {
    /*
     * Guardamos una copia del comando recibido.
     *
     * Puede ser:
     *
     * U, D, R, L, A o B.
     */
    comandoATmega = rxATmega;


    /*
     * Avisamos al while que hay un nuevo
     * comando disponible.
     */
    estadoUART = 1;


    /*
     * La funcion HAL_UART_Receive_IT recibe solamente
     * un byte cada vez.
     *
     * Por eso, despues de recibir un byte,
     * debemos volver a activar la recepcion para
     * esperar el siguiente comando.
     */
    HAL_UART_Receive_IT(&huart1, &rxATmega, 1);
  }
}


/* USER CODE END 4 */


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

  /* USER CODE BEGIN Error_Handler_Debug */


  /*
   */
  __disable_irq();


  while (1)
  {
  }


  /* USER CODE END Error_Handler_Debug */
}


#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{

  /* USER CODE BEGIN 6 */


  /*
   */


  /* USER CODE END 6 */
}

#endif /* USE_FULL_ASSERT */
