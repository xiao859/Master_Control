#include "task.h"

void TrigProcess()
{
    static uint8_t Trig_status = 0;
    if (Trig_status == 1)
        return;

    // === 1. start pluse ===
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);  // up
    uint32_t pulse_start = HAL_GetTick();

    // === 2. wait PC15 up()===
    uint32_t timeout = HAL_GetTick();
    while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET)
    {
        if (HAL_GetTick() - timeout > 10) break;  // max 10ms
    }

    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET)
    {
        // === 3. up running 3ms ===
        uint32_t high_start = HAL_GetTick();
        while (HAL_GetTick() - high_start < 3)
        {
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) != GPIO_PIN_SET)
                break;  // down,error
        }

        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET)
        {
            // === 4. wait down ===
            timeout = HAL_GetTick();
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET)
            {
                if (HAL_GetTick() - timeout > 10) break;  // max 10ms
            }

            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET)
            {
                if (cmdfifo.count < FRAME_BUF_NUM)
                {
                    // === success ===
                    uint8_t idx = cmdfifo.tail;
                    cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
                    cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
                    cmdfifo.data[idx].msg_id = CMD_TRIG;
                    cmdfifo.data[idx].data1 = 0x00;
                    cmdfifo.data[idx].data2 = 0x00;
                    cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
                    cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
                    cmdfifo.count++;
                    Trig_status = 1;
                }
            }
        }
    }
    // === 5. down PB8 end pouse ===
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

    if (Trig_status == 0)
    {
        HAL_Delay(5);  // delay 5ms
    }

}

void DoorProcess(void)
{
    static uint8_t doorstatus = 0;
    if (doorstatus == 1)
        return;

    GPIO_PinState current = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);

    // PC0
    GPIO_PinState new_level = (current == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, new_level);

    HAL_Delay(2);

    GPIO_PinState pb10_level = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10);

    if (pb10_level == new_level)
    {
        if (cmdfifo.count < FRAME_BUF_NUM)
        {
            // === success ===
            uint8_t idx = cmdfifo.tail;
            cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
            cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
            cmdfifo.data[idx].msg_id = CMD_DOOR;
            cmdfifo.data[idx].data1 = 0x00;
            cmdfifo.data[idx].data2 = 0x00;
            cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
            cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
            cmdfifo.count++;
        }
        doorstatus = 1;
    }
}

void XrayioProcess(void)
{
    static uint8_t io_status = 0;
    if (io_status >= 4)
        return;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_PinState prev, curr;
    switch (io_status)
    {
    case 0:
        //  PB10  output
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // PC0 input
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        //  PC0 status
        prev = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);

        // PB10 fanzhuan
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_10);
        HAL_Delay(2);  // wait

        //  PC0
        curr = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);

        if (curr != prev)
            io_status = 1;
        break;

    case 1:
        prev = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);

        // PA7
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
        HAL_Delay(2);
        curr = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
        if (prev != curr)
            io_status = 2;
        break;
    case 2:
        prev = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);

        // PA7
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
        HAL_Delay(2);
        curr = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
        if (prev != curr)
            io_status = 3;
        break;
    case 3:
        prev = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15);
        if (prev == GPIO_PIN_SET)
        {
            if (cmdfifo.count < FRAME_BUF_NUM)
            {
                // === success ===
                uint8_t idx = cmdfifo.tail;
                cmdfifo.data[idx].head1 = SCI_TES_HEADER1;
                cmdfifo.data[idx].head2 = SCI_TES_HEADER2;
                cmdfifo.data[idx].msg_id = CMD_XRAY_IO;
                cmdfifo.data[idx].data1 = 0x00;
                cmdfifo.data[idx].data2 = 0x00;
                cmdfifo.data[idx].checksum = uart_check(&cmdfifo.data[idx].msg_id);
                cmdfifo.tail = (cmdfifo.tail + 1) % FRAME_BUF_NUM;
                cmdfifo.count++;
            }
            io_status = 4;
        }
        break;
		default:
			break;
    }
}

void PhotoelectricSwitchProcess(void)
{

}

void FanProcess(void)
{

}


