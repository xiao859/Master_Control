#include "app_comm.h"
#include "string.h"

#define CMD_TIMEOUT_MS 3000

void UartProcess()
{
    if (uart5_frame_fifo.count == 0) 
			return;
		    uint8_t idx = uart5_frame_fifo.head;
        message_protocol* frame = & uart5_frame_fifo.data[idx];
        uint8_t cmd = frame->msg_id;

        switch (cmd)
        {
        case CMD_MOTOR1:
            
            break;
        case CMD_MOTOR2:
           
            break;
        case CMD_MOTOR_STOP:
            
            break;
        default:
           break;
        }
        uart5_frame_fifo.head = (uart5_frame_fifo.head + 1) % FRAME_BUF_NUM;
        uart5_frame_fifo.count--;

}


void CmdProcess(void)
{
    if (cmdfifo.count == 0) return;

    uint32_t now = HAL_GetTick();

    // wait ACK
    if (cmdfifo.waiting_ack)
    {
        if (now - cmdfifo.last_send_tick >= CMD_TIMEOUT_MS)
        {
            // timeout resend
            UART_Send_CMD(&cmdfifo.data[cmdfifo.head]);
            cmdfifo.last_send_tick = now;
        }
    }
    else
    {
        // send cmd
        UART_Send_CMD(&cmdfifo.data[cmdfifo.head]);
        cmdfifo.last_send_tick = now;
        cmdfifo.waiting_ack = 1;
    }
		return;
}




