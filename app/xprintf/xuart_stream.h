/**
  ******************************************************************************
  * @file           : xuart_stream.h
  * @author         : Rusanov M.N.
  * @version        : V1.0.0
  * @date           : 14-May-2024
  * @brief          : Header for xuart_stream.cpp file.
  *                   This file contains functions for working with the xprintf
  *                   library stream.
  * @note           : The library has not worked out thread-safety.
  *                   It is safe to call 'xprintf' only from one thread.
  *
  ******************************************************************************
  */

#pragma once

#include "main.h"
#include "xprintf.h"

class xuart_stream
{
public:
  enum class status
  {
    OK,
    ERROR
  };

  [[nodiscard]] static xuart_stream& get_instance();
  void init(UART_HandleTypeDef& huart);

#if XF_USE_OUTPUT
  void output_stream(char c);
#endif

#if XF_USE_INPUT
  [[nodiscard]] char input_stream() const;
#endif

private:
  explicit xuart_stream();

#if XF_USE_OUTPUT
  [[nodiscard]] status transmit_data(const uint8_t &data, uint16_t size) const;
  [[nodiscard]] status add_char(char c);
  [[nodiscard]] status add_endl();
#endif

private:
  static constexpr uint16_t tx_buf_size = 32;
  static constexpr char str_terminate_char = '\r'; // '\r' or '\n'
  UART_HandleTypeDef* f_huart = nullptr;

#if XF_USE_OUTPUT
  uint32_t f_max_transmission_time_ms = 0;
  uint8_t f_tx_buf[tx_buf_size] = { 0 };
  uint16_t f_tx_buf_idx = 0;
#endif
};