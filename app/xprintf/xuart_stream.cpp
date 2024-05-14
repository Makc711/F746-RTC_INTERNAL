/**
  ******************************************************************************
  * @file           : xuart_stream.cpp
  * @author         : Rusanov M.N.
  ******************************************************************************
  */

#include "xuart_stream.h"

void std_out(int c);
int std_in();

xuart_stream::xuart_stream()
{
#if XF_USE_OUTPUT
  xdev_out(std_out);
#endif

#if XF_USE_INPUT
  xdev_in(std_in);
#endif
}

xuart_stream& xuart_stream::get_instance()
{
  static xuart_stream instance;
  return instance;
}

void xuart_stream::init(UART_HandleTypeDef& huart)
{
  f_huart = &huart;

#if XF_USE_OUTPUT
  f_tx_buf[0] = 0;
  f_tx_buf_idx = 0;
#endif
}

#if XF_USE_OUTPUT
void std_out(const int c)
{
  xuart_stream::get_instance().output_stream(static_cast<char>(c));
}

void xuart_stream::output_stream(const char c)
{
  if (c != str_terminate_char)
  {
    if (add_char(c) != status::OK)
    {
      Error_Handler();
    }
  }
  else
  {
    if (add_endl() != status::OK)
    {
      Error_Handler();
    }
  }
}

xuart_stream::status xuart_stream::add_char(const char c)
{
  f_tx_buf[f_tx_buf_idx++] = c;

  if (f_tx_buf_idx >= tx_buf_size)
  {
    if (HAL_UART_Transmit(f_huart, f_tx_buf, f_tx_buf_idx, 100) != HAL_OK)
    {
      return status::ERROR;
    }
    f_tx_buf_idx = 0;
  }

  return status::OK;
}

xuart_stream::status xuart_stream::add_endl()
{
  if (add_char(str_terminate_char) != status::OK)
  {
    return status::ERROR;
  }

  const uint16_t len = f_tx_buf_idx;
  f_tx_buf_idx = 0;

  if (HAL_UART_Transmit(f_huart, f_tx_buf, len, 100) != HAL_OK)
  {
    return status::ERROR;
  }

  return status::OK;
}
#endif

#if XF_USE_INPUT
int std_in()
{
  return xuart_stream::get_instance().input_stream();
}

char xuart_stream::input_stream() const
{
  char c;

  if (HAL_UART_Receive(f_huart, reinterpret_cast<uint8_t*>(&c), 1, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }

  return c;
}
#endif