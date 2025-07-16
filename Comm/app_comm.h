#ifndef APP_COMM_H
#define APP_COMM_H

#include "stdint.h"
#include "app_uart.h"

// 简化后的电机配置结构体
typedef struct {
    uint8_t RunDir;      // 电机运动方向
    uint8_t PIN_Step;    // 步进引脚
    uint8_t PIN_Dir;     // 方向引脚
    uint32_t PlsPerCir;  // 每圈脉冲数
    float RPM;           // 电机转速
} motorConfig_t;

// 简化后的电机系统状态结构体
typedef struct {
    uint32_t RemainSteps; // 剩余步数
    uint32_t CurrentSteps;// 当前步数
    uint32_t CurFrq;      // 当前频率
    int32_t location;     // 电机当前位置
} motorSystem_t;

// 简化后的电机控制结构体
typedef struct {
    motorConfig_t config;
    motorSystem_t system;
    void (*TimeStart)(void);  // 启动定时器
    void (*TimeStop)(void);   // 停止定时器
} motorStepper_t;




void UartProcess(void);
void CmdProcess(void);
#endif

