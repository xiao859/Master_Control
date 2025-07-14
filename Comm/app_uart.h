#ifndef _APP_USART_H
#define _APP_USART_H

#include "stdio.h"
#include "main.h"


#define USART_BUFFER_LEN        100               /* max */
#define CMD_NUM                 20

#define MESSAGE_PACK_LENGTH     5

#define SCI_TES_HEADER1         0xAA
#define SCI_TES_HEADER2         0x55
#define TES_ACK_HEADER1         0x55
#define TES_ACK_HEADER2         0xAA

#define FRAME_BUF_NUM      10
typedef struct
{
    uint8_t head1;
    uint8_t head2;
    uint8_t msg_id;
    uint8_t data1;
		uint8_t data2;//ACK
    uint8_t checksum;
} message_protocol;

typedef struct
{
    message_protocol data[FRAME_BUF_NUM]; //
    uint8_t head; //
    uint8_t tail; //
    uint8_t count; //
} UARTFIFO_t;

typedef struct
{
    uint8_t     recv_byte;
    uint32_t    recv_len;
    uint8_t     recv_complete;
    uint32_t    receiving;
    uint32_t    uart_rx_cnt;
    uint8_t     uart_rx_buf[USART_BUFFER_LEN];
    uint8_t     uart_tx_buf[USART_BUFFER_LEN];
} UART_FILE;

extern UART_FILE uart1;
extern UART_FILE uart2 ;
extern UART_FILE uart3 ;
extern UART_FILE uart4 ;


extern UARTFIFO_t uart5_frame_fifo ;
extern UARTFIFO_t uart2_frame_fifo ;

typedef struct
{
    message_protocol data[CMD_NUM]; //
    uint8_t head; //
    uint8_t tail; //
    uint8_t count; //
	  uint32_t last_send_tick; // tick
    uint8_t waiting_ack;     // ACK
} CMDFIFO;

extern CMDFIFO cmdfifo;
extern uint8_t cmd_ack;

#define MSG_SUCCESS               0x00
#define MSG_ERROR                 0x01
#define MSG_LIMIT                 0x02
#define MSG_ACK										0x03

typedef enum
{
    CMD_BLANK = 0x00,
		CMD_TRIG,
    CMD_DOOR,
    CMD_XRAY_IO,
    CMD_XRAY_UART,
    CMD_PC_UART,
    CMD_TOUCH_UART,
    CMD_FAN,
    CMD_DEVICE,
    CMD_PE_SW,//Photoelectric switch
		CMD_MOTOR1,
		CMD_MOTOR2,	
    CMD_MOTOR_FORWORD,
    CMD_MOTOR_REVERSW,
    CMD_MOTOR_STOP,
    CMD_24V,
    CMD_5V,
    CMD_3_3V,
    CMD_TEST_1,
    CMD_TEST_2,
} CMD_ID;



uint8_t uart_check(uint8_t* data);

//void send_touch_message(uint8_t msg_id, uint8_t data1);
//void send_zk_message(uint8_t msg_id, uint8_t data1);
void UART_Send_CMD(message_protocol *cmd);
#endif


