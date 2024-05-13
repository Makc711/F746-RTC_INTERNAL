/**
  ******************************************************************************
  * @file           : rtc_internal.h
  * @author         : Rusanov M.N.
  * @version        : V1.0.0
  * @date           : 13-May-2024
  * @brief          : Header for rtc_internal.cpp file.
  *                   This file contains functions for working with RTC STM32
  *                   using UART.
  *
  ******************************************************************************
  */

#pragma once

#include "main.h"
#include "static_string.h"

extern UART_HandleTypeDef huart1;

class rtc_internal final
{
public:
  static rtc_internal& get_instance();
  void check_time_out_reception();
  static void uart_rx_cplt_callback(UART_HandleTypeDef* huart);
  void parse_received_msg();
  static void set_time(const char* str);
  static void set_date(const char* str);
  static void print_time();

private:
  enum class rtc_res
  {
    OK,
    WRONG_TIME,
    WRONG_DATE
  };

  explicit rtc_internal();
  void start_receive_msg();
  void restart_msg_reception();
  static rtc_res fix_time(RTC_TimeTypeDef& time, bool set_max);
  static rtc_res fix_date(RTC_DateTypeDef& date, bool set_max);

private:
  UART_HandleTypeDef& f_huart = huart1;
  static constexpr size_t rx_buf_size = 17;
  static constexpr auto cmd_set_t = snw1::STOSS("SET_T ");
  static constexpr auto cmd_set_d = snw1::STOSS("SET_D ");
  static constexpr auto cmd_get = snw1::STOSS("GET");
  const uint32_t f_reception_time_ms; // Maximum time for incoming msg

  uint8_t f_rx_buf[rx_buf_size] = { '\0' };
  size_t f_rx_buf_index = 0;
  char f_rx_msg[rx_buf_size] = { '\0' };
};
