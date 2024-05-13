/**
  ******************************************************************************
  * @file           : uart_stream.h
  * @author         : Rusanov M.N.
  * @version        : V1.0.1
  * @date           : 13-May-2024
  * @brief          : Header for uart_stream.cpp file.
  *                   This file contains functions for working with the standard
  *                   I/O stream.
  * @note           : The library has not worked out thread-safety.
  *                   It is safe to call 'printf' only from one thread.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_STREAM_H
#define __UART_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Global defines ------------------------------------------------------------*/
#define USE_OUTPUT_STREAM 1
#define USE_INPUT_STREAM  0
#define USE_N             0 // 1 - use '\n', 0 - don't use '\n'
#define USE_R             1 // 1 - use '\r', 0 - don't use '\r'
#define TERMINATING_CHAR '\r'

/* Global function prototypes ------------------------------------------------*/
void uart_stream_init(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __UART_STREAM_H */