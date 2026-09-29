# STM32-PWM-ADC-UART-Control
An STM32-based embedded control system that integrates **PWM generation, ADC measurement, relay control, and UART communication** using the STM32 HAL library.

The project demonstrates how an STM32 microcontroller can receive commands through UART, control PWM output, switch a relay, and acquire analog data using ADC — with **both DMA and Interrupt modes available and switchable at compile time**.

## Features

* PWM generation using **TIM1 Channel 1**
* PWM duty-cycle control from **0–100%**
* Analog voltage measurement using **ADC1**
* **Selectable ADC1 acquisition mode**: DMA or Interrupt, chosen via a single `#define` (`ADC_USE_DMA`) in `main.h`
* Relay ON/OFF control through **GPIO**
* UART communication using **USART3** at **9600 baud**
* **Selectable UART3 transfer mode**: DMA or Interrupt, chosen via a single `#define` (`UART_USE_DMA`) in `main.h`
  * Interrupt mode: `HAL_UART_Receive_IT()` / `HAL_UART_Transmit_IT()`
  * DMA mode: `HAL_UARTEx_ReceiveToIdle_DMA()` / `HAL_UART_Transmit_DMA()`
* Command-based control through serial communication, parsed by a single shared `Process_Command()` function (used by both DMA and Interrupt modes, so the command logic is written only once)
* STM32 HAL-based firmware architecture

## System Overview

The firmware provides three main control functions:

```text
             UART / Serial
                   │
                   ▼
            ┌───────────────┐
            │     STM32     │
            │               │
            │   USART3      │
            │      │        │
            │      ▼        │
            │ Command Parser│
            │      │        │
            └──────┼────────┘
                   │
        ┌──────────┼───────────┐
        │          │           │
        ▼          ▼           ▼
      PWM        Relay        ADC
   TIM1 CH1       PA5         ADC1
        │          │           │
        ▼          ▼           ▼
   PWM Output   Relay Output  Analog Input
```

## Hardware Configuration

| Peripheral | Configuration    | Function                              |
| ---------- | ---------------- | -------------------------------------- |
| USART3     | 9600 baud, 8N1   | Serial communication                   |
| TIM1 CH1   | PWM              | PWM output                             |
| ADC1       | Channel 4        | Analog measurement                     |
| DMA1       | ADC DMA (Ch.1)   | ADC data transfer (when `ADC_USE_DMA=1`) |
| DMA1       | USART3 RX (Ch.3) / TX (Ch.2) | UART data transfer (when `UART_USE_DMA=1`) |
| GPIO PA5   | Output (label `RELAY`) | Relay control                    |
| GPIO       | TIM1 CH1 output  | PWM signal                             |

> The DMA channels for USART3 are provided in the `.ioc` so that switching `UART_USE_DMA` to `1` works without any further CubeMX reconfiguration.

## Mode Configuration (`main.h`)

Both the UART and ADC transfer methods are controlled from two `#define` switches at the top of `main.h`:

```c
#define UART_USE_DMA    0   /* 0 = Interrupt (default) | 1 = DMA  */
#define ADC_USE_DMA     1   /* 0 = Interrupt           | 1 = DMA (default) */
```

Only **one** mode should be active per peripheral. Setting a flag to `1` or `0` automatically compiles in the corresponding code path (`#if` / `#else`) and compiles out the other one — there is no need to manually comment code in or out.

* `UART_USE_DMA = 1` requires the USART3 DMA requests (RX/TX) to be enabled in the `.ioc`, already included in this project.
* `ADC_USE_DMA = 1` requires the ADC1 DMA request, already included in this project.

## UART Commands

The controller accepts commands through USART3.

### PWM Control

The PWM duty cycle can be set using:

```text
PWM_0
PWM_10
PWM_25
PWM_50
PWM_75
PWM_100
```

The firmware extracts the numeric value after `PWM_` and accepts values between **0 and 100**.

For example:

```text
PWM_50
```

sets the PWM compare value to:

```text
50 × 10 = 500
```

With a timer period of 1000 counts, this corresponds approximately to a **50% duty cycle**.

### Relay Control

Turn the relay ON:

```text
relay ON
```

Turn the relay OFF:

```text
relay OFF
```

The relay is controlled through pin `PA5` (defined as `RELAY_Pin` / `RELAY_GPIO_Port` in `main.h`).

## PWM Configuration

TIM1 is configured with:

```c
Prescaler = 72 - 1;
Period = 1000 - 1;
```

With a 72 MHz timer clock, the resulting PWM frequency is approximately:

```text
72 MHz / 72 / 1000 = 1 kHz
```

Therefore, the PWM output operates at approximately **1 kHz**.

The duty cycle is controlled from 0–100%:

```c
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value * 10);
```

Example:

| Command   | Duty Cycle | Compare |
| --------- | ---------: | ------: |
| `PWM_0`   |         0% |       0 |
| `PWM_25`  |        25% |     250 |
| `PWM_50`  |        50% |     500 |
| `PWM_75`  |        75% |     750 |
| `PWM_100` |       100% |    1000 |

## ADC Configuration

ADC1 uses:

```text
ADC Channel 4
```

The ADC operates in continuous conversion mode. The conversion result is acquired using **either DMA or Interrupt**, depending on `ADC_USE_DMA`:

* **DMA mode** (`ADC_USE_DMA = 1`): the result is written automatically into `adc_dma_buffer[]` by `HAL_ADC_Start_DMA()`.
* **Interrupt mode** (`ADC_USE_DMA = 0`): the result is read inside `HAL_ADC_ConvCpltCallback()` via `HAL_ADC_GetValue()`. Because `ContinuousConvMode` is enabled, the next conversion starts automatically without calling `HAL_ADC_Start_IT()` again.

Either way, the latest reading is stored in:

```c
static volatile uint16_t adc_value;
```

and can be transmitted through UART, for example:

```text
ADC Value: 2048
```

## UART Communication

USART3 is configured as:

```text
Baud Rate : 9600
Data      : 8 bits
Parity    : None
Stop Bits : 1
Mode      : TX/RX
```

Depending on `UART_USE_DMA`:

* **Interrupt mode** (`UART_USE_DMA = 0`, default): characters are received one byte at a time via `HAL_UART_Receive_IT()` and accumulated in a buffer until a newline character (`\n`) is received.
* **DMA mode** (`UART_USE_DMA = 1`): a full line is received via `HAL_UARTEx_ReceiveToIdle_DMA()`, which completes automatically once the line goes idle — no per-byte interrupt handling needed.

In both modes, the completed line is handed to the same `Process_Command()` function, keeping the command-parsing logic identical regardless of the transfer mode.

## Relay Control

The relay is connected to `PA5` (labeled `RELAY` in the `.ioc`, exposed in `main.h` as `RELAY_Pin` / `RELAY_GPIO_Port`).

Relay ON:

```c
HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
```

Relay OFF:

```c
HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);
```

The firmware also sends a status message through UART:

```text
Relay ON
```

or:

```text
Relay OFF
```

## Development Environment

* **STM32CubeIDE**
* **STM32CubeMX**
* **STM32 HAL Library**
* **C Programming Language**

## Main Technologies

```text
STM32
Embedded C
STM32 HAL
ADC
DMA
PWM
Timer
UART
GPIO
Interrupt
Relay Control
```

## Example Application

This project can be used as a foundation for embedded control applications such as:

* Motor speed control
* DC power control
* Actuator control
* Relay switching
* Analog sensor monitoring
* Power electronics control
* Industrial control systems
* UART-based device control
* IoT and automation controllers

## Notes

The current firmware is intended as a development and demonstration project. Hardware-specific considerations such as relay driver circuitry, electrical isolation, load current, protection components, and ADC signal conditioning should be evaluated before connecting the controller to an actual industrial or high-power load.

## Author

**Kurnia Aditya Reynaldi**

Electrical Engineer | Embedded Systems | Control Systems | Electronics R&D

Contributions, issues, and pull requests are welcome.

---

## License

This project can be distributed and modified according to the license selected for this repository.
