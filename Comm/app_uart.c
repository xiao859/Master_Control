#include "app_uart.h"
#include "stm32f4xx_hal.h"
//#include "update.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*   uart1    PC
     uart2    xray
     uart3    motor1
     uart4    motor2
     uart5    touch
*/

UART_FILE uart1 = {0};
UART_FILE uart2 = {0};
UART_FILE uart3 = {0};
UART_FILE uart4 = {0};
UART_FILE uart5 = {0};


UARTFIFO_t uart5_frame_fifo = {0};

CMDFIFO cmdfifo = {0};

static uint8_t uarttemp[6] = {0x55, 0xaa, 0xa5, 0x5a, 0x00, 0xff};

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)             /*  */
    {
        uint8_t d = uart5.recv_byte;

        if (!uart5.receiving)
        {
            if (d == TES_ACK_HEADER1)
            {
                uart5.uart_rx_buf[0] = d;
                uart5.uart_rx_cnt = 1;
                uart5.receiving = 1;
            }
        }
        else
        {
            uart5.uart_rx_buf[uart5.uart_rx_cnt++] = d;

            if (uart5.uart_rx_cnt >= 6)
            {
                uint8_t check_recv = uart_check(&(uart5.uart_rx_buf[2]));
                if (check_recv == 1)
                {
                    if (memcmp(uart1.uart_rx_buf, uarttemp, 6))
                    {
                        uint8_t idx = cmdfifo.tail;
                        cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
                        cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
                        cmdfifo.data[idx].msg_id = CMD_TOUCH_UART;
                        cmdfifo.data[idx].data1 = 0x00;
                        cmdfifo.data[idx].data2 = 0x00;
                        cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
                        cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
                        cmdfifo.count++;
                    }
                    else if (uart5.uart_rx_buf[4] == MSG_ACK)//ACK
                    {
                        cmdfifo.head = (cmdfifo.head + 1) % CMD_NUM;
                        cmdfifo.count--;
                        cmdfifo.waiting_ack = 0;
                    }
                    // FIFO
                    else if (uart5_frame_fifo.count < FRAME_BUF_NUM)
                    {
                        uint8_t idx = uart5_frame_fifo.tail;
                        memcpy(&uart5_frame_fifo.data[idx], uart5.uart_rx_buf, 6);
                        uart5_frame_fifo.tail = (uart5_frame_fifo.tail + 1) % FRAME_BUF_NUM;
                        uart5_frame_fifo.count++;
                    }
                }
                uart5.receiving = 0;
                uart5.uart_rx_cnt = 0;
            }
        }
        HAL_UART_Receive_IT(&huart5, &uart5.recv_byte, 1);
    }
    else if (huart->Instance == USART1)//pc
    {
        uint8_t d = uart1.recv_byte;

        if (!uart1.receiving)
        {
            if (d == TES_ACK_HEADER1)
            {
                uart1.uart_rx_buf[0] = d;
                uart1.uart_rx_cnt = 1;
                uart1.receiving = 1;
            }
        }
        else
        {
            uart1.uart_rx_buf[uart1.uart_rx_cnt++] = d;
            if (uart1.uart_rx_cnt >= 6)
            {
                if (memcmp(uart1.uart_rx_buf, uarttemp, 6))
                {
                    uint8_t idx = cmdfifo.tail;
                    cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
                    cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
                    cmdfifo.data[idx].msg_id = CMD_PC_UART;
                    cmdfifo.data[idx].data1 = 0x00;
                    cmdfifo.data[idx].data2 = 0x00;
                    cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
                    cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
                    cmdfifo.count++;
                }
                uart1.receiving = 0;
                uart1.uart_rx_cnt = 0;
            }
        }
        HAL_UART_Receive_IT(&huart1, &uart1.recv_byte, 1);
    }
    else if (huart->Instance == USART2)               /* xray */
    {
        uint8_t d = uart2.recv_byte;

        if (!uart2.receiving)
        {
            if (d == TES_ACK_HEADER1)
            {
                uart2.uart_rx_buf[0] = d;
                uart2.uart_rx_cnt = 1;
                uart2.receiving = 1;
            }
        }
        else
        {
            uart2.uart_rx_buf[uart2.uart_rx_cnt++] = d;
            if (uart2.uart_rx_cnt >= 6)
            {
                if (memcmp(uart2.uart_rx_buf, uarttemp, 6))
                {
                    uint8_t idx = cmdfifo.tail;
                    cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
                    cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
                    cmdfifo.data[idx].msg_id = CMD_XRAY_UART;
                    cmdfifo.data[idx].data1 = 0x00;
                    cmdfifo.data[idx].data2 = 0x00;
                    cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
                    cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
                    cmdfifo.count++;
                }
                uart2.receiving = 0;
                uart2.uart_rx_cnt = 0;
            }
        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&uart2.recv_byte, 1);
    }

    else if (huart->Instance == USART3)
    {


        HAL_UART_Receive_IT(&huart3, (uint8_t *)&uart3.recv_byte, 1);
    }
    else if (huart->Instance == UART4)
    {
        HAL_UART_Receive_IT(&huart4, (uint8_t *)&uart4.recv_byte, 1);
    }
}

//void restart_usart_receive(USART_TypeDef *Instance)
//{
//    if (Instance == USART6)
//    {
//        uart6.recv_complete = 0;
//        uart6.recv_len = 0;
//        uart6.uart_rx_cnt = 0;
//        HAL_UART_Receive_IT(&huart6, (uint8_t *)&uart6.recv_byte, 1);
//    }

//    return;
//}

//void send_touch_message(uint8_t msg_id, uint8_t data1)
//{
//    message_protocol msg_reply;
//    msg_reply.head1 = TOU_ACK_HEADER1;
//    msg_reply.head2 = TOU_ACK_HEADER2;
//    msg_reply.msg_id = msg_id;
//    msg_reply.data1  = data1;
//    msg_reply.checksum = (0 - (msg_reply.msg_id + msg_reply.data1)) & 0xFF;

//    memcpy((uint8_t *)uart1.uart_tx_buf, &msg_reply, MESSAGE_PACK_LENGTH);
//    HAL_UART_Transmit(&huart1, (uint8_t*)uart1.uart_tx_buf, MESSAGE_PACK_LENGTH, 1000);
//}

//void send_zk_message(uint8_t msg_id, uint8_t data1)
//{
//    message_protocol msg_reply;
//    msg_reply.head1 = TES_ACK_HEADER1;
//    msg_reply.head2 = TES_ACK_HEADER2;
//    msg_reply.msg_id = msg_id;
//    msg_reply.data1  = data1;
//    msg_reply.checksum = (0 - (msg_reply.msg_id + msg_reply.data1)) & 0xFF;

//    memcpy((uint8_t *)uart1.uart_tx_buf, &msg_reply, MESSAGE_PACK_LENGTH);
//    HAL_UART_Transmit(&huart1, (uint8_t*)uart1.uart_tx_buf, MESSAGE_PACK_LENGTH, 1000);
//}

uint8_t uart_check(uint8_t* data)
{
    uint16_t sum = 0;
    sum = sum + data[0] + data[1] + data[2];

    if ((sum & 0xFF) == 0x00)
        return 1;
    else
        return 0;
}

void UART_Send_CMD(message_protocol *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, sizeof(message_protocol), 100);
}



