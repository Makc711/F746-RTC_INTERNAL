/**
  ******************************************************************************
  * @file           : uart_stream.h
  * @author         : Rusanov M.N.
  * @version        : V1.0.0
  * @date           : 11-May-2024
  * @brief          : Header for uart_stream.cpp file.
  *                   This file contains functions for working with the standard
  *                   I/O stream.
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

/* Global function prototypes ------------------------------------------------*/
void uart_stream_init(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __UART_STREAM_H */