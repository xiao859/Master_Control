#include "app_comm.h"
#include "string.h"
#include "stdbool.h"

#define CMD_TIMEOUT_MS 3000

#define MOTOR1_STEP_PIN     GPIO_PIN_0
#define MOTOR1_DIR_PIN      GPIO_PIN_1
#define MOTOR2_STEP_PIN     GPIO_PIN_2
#define MOTOR2_DIR_PIN      GPIO_PIN_3
#define MOTOR_GPIO_PORT     GPIOA  // 修改为你所使用的 GPIO 端口


// 电机控制函数：正转、反转、停止
typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
} MotorState;

MotorState motor_state = MOTOR_STOP;

void Motor_Control(MotorState state)
{
    switch (state)
    {
    case MOTOR_FORWARD:
        HAL_GPIO_WritePin(MOTOR_GPIO_PORT, MOTOR2_DIR_PIN, GPIO_PIN_SET); // 设置正转方向
        break;
    case MOTOR_BACKWARD:
        HAL_GPIO_WritePin(MOTOR_GPIO_PORT, MOTOR2_DIR_PIN, GPIO_PIN_RESET); // 设置反转方向
        break;
    case MOTOR_STOP:
        break;
    }
}


extern uint8_t fan_status, dev_led_status;

void UartProcess()//还需加上timer启动和停止
{
    if (uart5_frame_fifo.count == 0)
        return;
    uint8_t idx = uart5_frame_fifo.head;
    message_protocol* frame = & uart5_frame_fifo.data[idx];
    uint8_t cmd = frame->msg_id;

    switch (cmd)
    {
    case CMD_MOTOR_FORWORD:
        motor_state = MOTOR_FORWARD;
        Motor_Control(motor_state);
        break;
    case CMD_MOTOR_REVERSW:
        motor_state = MOTOR_BACKWARD;
        Motor_Control(motor_state);
        break;
    case CMD_MOTOR_STOP:
        motor_state = MOTOR_STOP;
        Motor_Control(motor_state);
        break;
    case CMD_FAN:
        if (frame->data1 == 0)
            fan_status = 1;
        if (frame->data1 == 1)
            fan_status = 3;
    case CMD_DEVICE:
        if (frame->data1 == 1)
            dev_led_status = 1;
        if (frame->data1 == 1)
            dev_led_status = 3;
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




