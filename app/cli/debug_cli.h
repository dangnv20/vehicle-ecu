/*
 * debug_cli.h
 *
 *  Created on: Nov 1, 2026
 *      Author: vandang
 */

#ifndef DEBUG_CLI_H
#define DEBUG_CLI_H

#include <stdint.h>

void Cli_Init(void);
/* Hàm nhận từng byte ký tự từ ngắt UART đưa vào bộ đệm dòng lệnh */
void Cli_ReceiveChar(char rx_char);
/* Hàm chạy trong CliTask (chu kỳ thấp hoặc event-driven) để xử lý lệnh khi nhấn Enter */
void Cli_Process(void);

#endif /* DEBUG_CLI_H */
