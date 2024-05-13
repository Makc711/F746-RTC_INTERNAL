/* Includes ------------------------------------------------------------------*/
#include "uart_stream.h"
#include <stdio.h>
#include <errno.h>

/* Private defines -----------------------------------------------------------*/
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef* g_huart;

/* Global function -----------------------------------------------------------*/
void uart_stream_init(UART_HandleTypeDef *huart)
{
  g_huart = huart;

  /* Disable I/O buffering for STDOUT stream, so that
     * chars are sent out as soon as they are printed. */
  setvbuf(stdout, NULL, _IONBF, 0);
}

#if USE_OUTPUT_STREAM
int _write(int fd, char* ptr, int len) 
{
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) 
  {
    return (HAL_UART_Transmit(g_huart, (uint8_t*)ptr, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK)
            ? len
            : EIO;
  }
  errno = EBADF;
  return -1;
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