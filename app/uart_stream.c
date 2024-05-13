/* Includes ------------------------------------------------------------------*/
#include "uart_stream.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define TX_BUF_SIZE 32

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef* g_huart;
static uint8_t g_tx_buf[TX_BUF_SIZE] = { 0 };
static uint16_t g_tx_buf_idx = 0;

/* Global function -----------------------------------------------------------*/
void uart_stream_init(UART_HandleTypeDef *huart)
{
  g_huart = huart;

  /* Disable I/O buffering for STDOUT stream, so that
     * chars are sent out as soon as they are printed. */
  setvbuf(stdout, NULL, _IONBF, 0);
}

#if USE_OUTPUT_STREAM
static inline int _add_char(const char c)
{
  g_tx_buf[g_tx_buf_idx++] = c;

  if (g_tx_buf_idx >= TX_BUF_SIZE)
  {
    if (HAL_UART_Transmit(g_huart, g_tx_buf, g_tx_buf_idx, 100) != HAL_OK)
    {
      errno = EIO;
      return -1;
    }
    g_tx_buf_idx = 0;
  }

  return 0;
}

static inline int _add_endl()
{
#if USE_R
  if (_add_char('\r') != 0)
  {
    return -1;
  }
#endif

#if USE_N
  if (_add_char('\n') != 0)
  {
    return -1;
  }
#endif

  const uint16_t len = g_tx_buf_idx;
  g_tx_buf_idx = 0;

  if (HAL_UART_Transmit(g_huart, g_tx_buf, len, 100) != HAL_OK)
  {
    errno = EIO;
    return -1;
  }

  return 0;
}

int _write(int fd, char* data, int len)
{
  if ((fd != STDOUT_FILENO) && (fd != STDERR_FILENO))
  {
    errno = EBADF;
    return -1;
  }

  if (*data != TERMINATING_CHAR)
  {
    if (_add_char(*data) != 0)
    {
      return -1;
    }
  }
  else
  {
    if (_add_endl() != 0)
    {
      return -1;
    }
  }

  return 1;
}
#endif

#if USE_INPUT_STREAM
int _read(int fd, char* ptr, int len)
{
  if (fd == STDIN_FILENO)
  {
    return (HAL_UART_Receive(g_huart, (uint8_t*)ptr, 1, HAL_MAX_DELAY) == HAL_OK)
            ? 1
            : EIO;
  }
  errno = EBADF;
  return -1;
}
#endif