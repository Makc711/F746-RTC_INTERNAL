/**
  ******************************************************************************
  * @file           : rtc_internal.cpp
  * @author         : Rusanov M.N.
  ******************************************************************************
  */

#include "rtc_internal.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include "xprintf.h"

extern RTC_HandleTypeDef hrtc;

rtc_internal::rtc_internal() = default;

rtc_internal& rtc_internal::get_instance()
{
  static rtc_internal instance;
  return instance;
}

void rtc_internal::init(UART_HandleTypeDef& huart)
{
  f_huart = &huart;
  f_max_reception_time_ms = (rx_buf_size * (1 + 8 + 2) * 1000 / huart.Init.BaudRate + 2) * 3;
  start_receive_msg();
}

void rtc_internal::initiate_reception()
{
  HAL_UART_Receive_IT(f_huart, const_cast<uint8_t*>(&f_rx_buf[f_rx_buf_index]), sizeof(f_rx_buf[0]));
}

void rtc_internal::start_receive_msg()
{
  f_rx_buf_index = 0;
  initiate_reception();
}

void rtc_internal::restart_msg_reception()
{
  HAL_UART_AbortReceive_IT(f_huart);
  start_receive_msg();
}

/**
  * @brief Must be called in the SysTick_Handler() interrupt handler with
  *        a period of 1 ms.
  */
void rtc_internal::check_time_out_reception()
{
  static uint32_t time_out = 0;
  if (f_rx_buf_index == 0)
  {
    time_out = 0; 
  }
  else 
  {
    ++time_out;
    if (time_out >= f_max_reception_time_ms) 
    {
      xprintf("Error: Timeout command!\r");
      time_out = 0;
      restart_msg_reception();
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  rtc_internal::get_instance().uart_rx_cplt_callback(huart);
}

void rtc_internal::uart_rx_cplt_callback(const UART_HandleTypeDef* huart)
{
  if (huart == f_huart)
  {
    forming_rx_msg();
  }
}

void rtc_internal::forming_rx_msg()
{
  if (f_rx_buf[f_rx_buf_index] != '\r')
  {
    f_rx_buf_index = f_rx_buf_index + 1;

    if (f_rx_buf_index >= rx_buf_size)
    {
      xprintf("Error: Msg size exceeded!\r");
      restart_msg_reception();
      return;
    }
  }
  else
  {
    if (f_rx_msg[0] != '\0') // If msg hasn't been parsed to this point.
    {
      execute_cmd(parse_received_msg()); // Start the parser forced!
    }

    std::copy_n(f_rx_buf, f_rx_buf_index, f_rx_msg);
    f_rx_msg[f_rx_buf_index] = '\0';
    f_rx_buf_index = 0;
  }

  initiate_reception();
}

/**
  * @brief  This function must be called in the main loop to parse received msg.
  * @note   This function parses msg received by UART into rtc_cmd and time/data str.
  * @retval The command of rtc_cmd and pointer to string of format @ref time_template
  *         or @ref data_template with result of time/data (ptr to location in the @ref f_rx_msg).
  */
rtc_internal::cmd_info rtc_internal::parse_received_msg()
{
  cmd_info result = { rtc_cmd::NONE, "" };

  if (f_rx_msg[0] != '\0')
  {
    if (std::strncmp(const_cast<const char*>(f_rx_msg), cmd_set_t.c_str(), cmd_set_t.length()) == 0)
    {
      result = { rtc_cmd::SET_T, const_cast<const char*>(f_rx_msg) + cmd_set_d.length() };
    }
    else if (std::strncmp(const_cast<const char*>(f_rx_msg), cmd_set_d.c_str(), cmd_set_d.length()) == 0)
    {
      result = { rtc_cmd::SET_D, const_cast<const char*>(f_rx_msg) + cmd_set_d.length() };
    }
    else if (std::strncmp(const_cast<const char*>(f_rx_msg), cmd_get.c_str(), cmd_get.length()) == 0)
    {
      if (f_rx_msg[cmd_get.length()] == '\0')
      {
        result.cmd = rtc_cmd::GET;
      }
      else
      {
        xprintf("Error: Wrong command!\r");
      }
    }
    else
    {
      xprintf("Error: Wrong command!\r");
    }

    f_rx_msg[0] = '\0';
  }

  return result;
}

void rtc_internal::execute_cmd(const cmd_info& data)
{
  switch (data.cmd)
  {
    case rtc_cmd::SET_T:
      set_time(data.res_str);
      break;
    case rtc_cmd::SET_D:
      set_date(data.res_str);
      break;
    case rtc_cmd::GET:
      print_time();
      break;
    case rtc_cmd::NONE:
      break;
  }
}

/**
  * @brief  Sets RTC current time.
  * @param  str : pointer to string of format @ref time_template
  */
void rtc_internal::set_time(const char* str)
{
  unsigned int hours = 0;
  unsigned int minutes = 0;
  unsigned int seconds = 0;

  if (sscanf(str, "%2u:%2u:%2u", &hours, &minutes, &seconds) == 3) 
  {
    RTC_TimeTypeDef time_set;
    time_set.Hours = static_cast<uint8_t>(hours);
    time_set.Minutes = static_cast<uint8_t>(minutes);
    time_set.Seconds = static_cast<uint8_t>(seconds);

    if (fix_time(time_set, true) != rtc_res::OK)
    {
      xprintf("Error: Wrong time! Maybe you mean: %02u:%02u:%02u?\r",
        time_set.Hours,
        time_set.Minutes,
        time_set.Seconds);
    }

    if (const auto res = HAL_RTC_SetTime(&hrtc, &time_set, RTC_FORMAT_BIN); 
        res != HAL_OK)
    {
      xprintf("Error %u: Failed to set time!\r", res);
    }
  }
  else 
  {
    xprintf("Error: Wrong time format!\r");
  }
}

/**
  * @brief  Sets RTC current date.
  * @param  str : pointer to string of format @ref data_template
  */
void rtc_internal::set_date(const char* str)
{
  unsigned int day = 0;
  unsigned int month = 0;
  unsigned int year = 0;

  if (sscanf(str, "%2u/%2u/%4u", &day, &month, &year) == 3) 
  {
    RTC_DateTypeDef date_set;
    date_set.Date = static_cast<uint8_t>(day);
    date_set.Month = static_cast<uint8_t>(month);
    date_set.Year = static_cast<uint8_t>(year % 100);

    if (fix_date(date_set, true) != rtc_res::OK)
    {
      xprintf("Error: Wrong date! Maybe you mean: %02u/%02u/%4u?\r",
        date_set.Date,
        date_set.Month,
        static_cast<unsigned int>(date_set.Year) + 2000);
    }

    if (const auto res = HAL_RTC_SetDate(&hrtc, &date_set, RTC_FORMAT_BIN); 
        res != HAL_OK)
    {
      xprintf("Error %u: Failed to set data!\r", res);
    }
  }
  else 
  {
    xprintf("Error: Wrong data format!\r");
  }
}

/**
  * @brief  Sends the current time and date to UART in format
  *         dd/mm/yyyyy hh:mm:ss\r.
  */
void rtc_internal::print_time()
{
  RTC_TimeTypeDef time_get;
  if (const auto res = HAL_RTC_GetTime(&hrtc, &time_get, RTC_FORMAT_BIN); 
      res != HAL_OK)
  {
    xprintf("Error %u: Failed to read time!\r", res);
  }

  RTC_DateTypeDef date_get;
  if (const auto res = HAL_RTC_GetDate(&hrtc, &date_get, RTC_FORMAT_BIN);
      res != HAL_OK)
  {
    xprintf("Error %u: Failed to read date!\r", res);
  }

  xprintf("%02u/%02u/%4u %02u:%02u:%02u\r", 
    date_get.Date, date_get.Month, 2000 + date_get.Year, 
    time_get.Hours, time_get.Minutes, time_get.Seconds);
}

rtc_internal::rtc_res rtc_internal::fix_time(RTC_TimeTypeDef& time, const bool set_max)
{
  auto result = rtc_res::OK;

  if (const uint8_t max_hour = (hrtc.Init.HourFormat == RTC_HOURFORMAT_24) ? 23 : 12; 
      time.Hours > max_hour)
  {
    time.Hours = set_max ? max_hour : 0;
    result = rtc_res::WRONG_TIME;
  }

  if (time.Minutes > 59)
  {
    time.Minutes = set_max ? 59 : 0;
    result = rtc_res::WRONG_TIME;
  }

  if (time.Seconds > 59)
  {
    time.Seconds = set_max ? 59 : 0;
    result = rtc_res::WRONG_TIME;
  }

  return result;
}

rtc_internal::rtc_res rtc_internal::fix_date(RTC_DateTypeDef& date, const bool set_max)
{
  auto result = rtc_res::OK;

  if (date.Month == 0)
  {
    date.Month = 1;
    result = rtc_res::WRONG_DATE;
  }

  if (date.Month > 12)
  {
    date.Month = set_max ? 12 : 1;
    result = rtc_res::WRONG_DATE;
  }

  if (date.Date == 0)
  {
    date.Date = 1;
    result = rtc_res::WRONG_DATE;
  }

  if (date.Date <= 28) 
  {
    return result;
  }

  uint8_t days_in_month = 31;

  if (date.Month == 2)
  {
    const bool is_leap_year = (date.Year % 4) == 0;
    days_in_month = is_leap_year ? 29 : 28;
  }
  else if ((date.Month == 4) || (date.Month == 6) || (date.Month == 9) || (date.Month == 11))
  {
    days_in_month = 30;
  }

  if (date.Date > days_in_month) 
  {
    date.Date = set_max ? days_in_month : 1;
    result = rtc_res::WRONG_DATE;
  }

  return result;
}

