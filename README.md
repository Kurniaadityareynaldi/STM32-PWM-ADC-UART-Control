# STM32-PWM-ADC-UART-Control
An STM32-based embedded control system that integrates **PWM generation, ADC measurement, relay control, and UART communication** using the STM32 HAL library.

The project demonstrates how an STM32 microcontroller can receive commands through UART, control PWM output, switch a relay, and acquire analog data using ADC with DMA.

## Features

* PWM generation using **TIM1 Channel 1**
* PWM duty-cycle control from **0–100%**
* Analog voltage measurement using **ADC1**
* ADC data acquisition using **DMA**
* Relay ON/OFF control through **GPIO**
* UART communication using **USART3**
* UART communication at **9600 baud**
* UART receive interrupt using `HAL_UART_Receive_IT()`
* UART transmit interrupt using `HAL_UART_Transmit_IT()`
* Command-based control through serial communication
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

| Peripheral | Configuration   | Function             |
| ---------- | --------------- | -------------------- |
| USART3     | 9600 baud, 8N1  | Serial communication |
| TIM1 CH1   | PWM             | PWM output           |
| ADC1       | Channel 4       | Analog measurement   |
| DMA1       | ADC DMA         | ADC data transfer    |
| GPIO PA5   | Output          | Relay control        |
| GPIO       | TIM1 CH1 output | PWM signal           |

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

The relay is controlled through:

```text
PA5
```

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

The ADC operates in continuous conversion mode and transfers the conversion result using DMA.

The ADC result is stored in:

```c
uint16_t readValue[1];
```

and copied to:

```c
uint16_t adc;
```

The firmware can then transmit the ADC result through UART.

Example output:

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

UART reception uses interrupt mode:

```c
HAL_UART_Receive_IT(&huart3, &rx_data, 1);
```

Received characters are accumulated in a buffer until a newline character (`\n`) is received.

The command is then parsed and processed.

## Relay Control

The relay is connected to:

```text
PA5
```

Relay ON:

```c
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
```

Relay OFF:

```c
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
```

The firmware also sends a status message through UART:

```text
Relay ON
```

or:

```text
Relay OFF
```

## Project Structure

Typical STM32CubeIDE project structure:

```text
STM32-PWM-ADC-UART-Control/
│
├── Core/
│   ├── Inc/
│   │   └── main.h
│   │
│   └── Src/
│       ├── main.c
│       ├── stm32f1xx_it.c
│       └── system_stm32f1xx.c
│
├── Drivers/
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
│
├── .ioc
└── README.md
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

Electrical Engineering | Embedded Systems | Control Systems | Electronics R&D

---

## License

This project can be distributed and modified according to the license selected for this repository.
