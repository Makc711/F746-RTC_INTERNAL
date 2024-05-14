/**
  ******************************************************************************
  * @file           : rtc_internal.h
  * @author         : Rusanov M.N.
  * @version        : V1.0.2
  * @date           : 14-May-2024
  * @brief          : Header for rtc_internal.cpp file.
  *                   This file contains functions for working with RTC STM32
  *                   using UART.
  *
  ******************************************************************************
  */

#pragma once

#include "main.h"
#include "static_string.h"

class rtc_internal
{
public:
  enum class rtc_cmd
  {
    SET_T,
    SET_D,
    GET,
    NONE
  };

  struct cmd_info
  {
    rtc_cmd cmd;
    const char* res_str;
  };

  [[nodiscard]] static rtc_internal& get_instance();
  void init(UART_HandleTypeDef& huart);
  void check_time_out_reception();
  void uart_rx_cplt_callback(UART_HandleTypeDef* huart);
  [[nodiscard]] cmd_info parse_received_msg();
  static void execute_cmd(const cmd_info& data);
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
  static constexpr auto cmd_set_t = snw1::STOSS("SET_T ");
  static constexpr auto cmd_set_d = snw1::STOSS("SET_D ");
  static constexpr auto cmd_get = snw1::STOSS("GET");
  static constexpr auto time_template = snw1::STOSS("hh:mm:ss");
  static constexpr auto data_template = snw1::STOSS("dd/mm/yyyy");
  static constexpr size_t rx_buf_size = snw1::max<cmd_set_t.length() + time_template.length(),
                                                  cmd_set_d.length() + data_template.length(),
                                                  cmd_get.length()>() + 1;
  UART_HandleTypeDef* f_huart = nullptr;
  uint32_t f_max_reception_time_ms = 0;
  uint8_t f_rx_buf[rx_buf_size] = { '\0' };
  size_t f_rx_buf_index = 0;
  char f_rx_msg[rx_buf_size] = { '\0' };
};
