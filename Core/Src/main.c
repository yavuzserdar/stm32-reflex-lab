/* STM32 Reflex Lab: NUCLEO-F446RE (MB1136), LD2=PA5, B1=PC13 active low. */
#include "main.h"
#include "reflex.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef uart;
static TIM_HandleTypeDef timer;
static volatile uint32_t edge_time;
static volatile uint8_t edge_pending, button_armed;
static Reflex game;

static uint32_t micros(void) { return __HAL_TIM_GET_COUNTER(&timer); }
static void led(uint8_t on) { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, on ? GPIO_PIN_SET : GPIO_PIN_RESET); }
static void send(const char *s)
{
    /* Only the foreground transmits; the ISR keeps capturing input timestamps. */
    if (HAL_UART_Transmit(&uart, (uint8_t *)s, (uint16_t)strlen(s), 100U) != HAL_OK)
        Error_Handler();
}

static void clock_init(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM = 16;
    osc.PLL.PLLN = 336;
    osc.PLL.PLLP = RCC_PLLP_DIV4;
    osc.PLL.PLLQ = 7;
    osc.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

static void peripherals_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    led(0);
    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* APB1 timer clock is 84 MHz: 84 / (83+1) = 1 MHz. */
    timer.Instance = TIM2;
    timer.Init.Prescaler = 83;
    timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    timer.Init.Period = UINT32_MAX;
    timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&timer) != HAL_OK || HAL_TIM_Base_Start(&timer) != HAL_OK) Error_Handler();

    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
    uart.Instance = USART2;
    uart.Init.BaudRate = 115200;
    uart.Init.WordLength = UART_WORDLENGTH_8B;
    uart.Init.StopBits = UART_STOPBITS_1;
    uart.Init.Parity = UART_PARITY_NONE;
    uart.Init.Mode = UART_MODE_TX_RX;
    uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&uart) != HAL_OK) Error_Handler();

    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_IT_FALLING;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void report(ReflexEvent event)
{
    char line[160];
    switch (event) {
    case EVENT_START:
        snprintf(line, sizeof(line), "# SESSION %lu: release B1; 10 valid rounds.\r\n", (unsigned long)game.session);
        send(line);
        break;
    case EVENT_WAIT: send("# WAIT: press B1 only when LD2 lights.\r\n"); break;
    case EVENT_GO: send("# GO\r\n"); break;
    case EVENT_EARLY: send("# EARLY: round repeated. Release B1.\r\n"); break;
    case EVENT_TIMEOUT: send("# TIMEOUT: round repeated.\r\n"); break;
    case EVENT_SCORE:
    case EVENT_COMPLETE:
        snprintf(line, sizeof(line), "RESULT,%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n",
                 (unsigned long)game.session, (unsigned long)game.valid,
                 (unsigned long)game.reaction, (unsigned long)game.best,
                 (unsigned long)(game.total / game.valid),
                 (unsigned long)game.early, (unsigned long)game.timeouts);
        send(line);
        if (event == EVENT_COMPLETE) send("# COMPLETE: press B1 for a new session.\r\n");
        break;
    default: break;
    }
}

int main(void)
{
    uint32_t release_since = 0;
    uint8_t releasing = 0;
    HAL_Init();
    clock_init();
    peripherals_init();
    Reflex_Init(&game);
    send("# STM32 Reflex Lab v1.0 | B1=start/react | LD2=GO\r\n");
    send("# RESULT,session,round,reaction_us,best_us,average_us,early,timeouts\r\n");
    send("# READY: press and release blue B1.\r\n");

    while (1) {
        /* Atomic input handoff and LED transition; no UART in this section. */
        __disable_irq();
        uint32_t now = micros();
        uint8_t press = edge_pending;
        uint32_t edge = edge_time;
        edge_pending = 0;
        if (press || HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
            releasing = 0;
        } else if (!releasing) {
            releasing = 1;
            release_since = now;
        }
        uint8_t released = releasing && (uint32_t)(now - release_since) >= 30000U;
        if (released) button_armed = 1;
        ReflexEvent event = Reflex_Step(&game, now, press != 0, edge, released != 0);
        if (event == EVENT_GO) {
            led(1);
            game.signal_at = micros();
        } else if (event == EVENT_SCORE || event == EVENT_COMPLETE || event == EVENT_TIMEOUT) {
            led(0);
        }
        __enable_irq();
        report(event);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == GPIO_PIN_13 && button_armed) {
        edge_time = micros();
        edge_pending = 1;
        button_armed = 0; /* Re-arm only after 30 ms of stable release. */
    }
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}
void SysTick_Handler(void) { HAL_IncTick(); }
void EXTI15_10_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); }
void NMI_Handler(void) { while (1) {} }
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}
void Error_Handler(void) { __disable_irq(); while (1) {} }
