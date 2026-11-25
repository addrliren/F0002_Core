# stm32prj

基于 **STM32F103ZET6**（Cortex-M3 / LQFP144）的工控盒固件，CubeMX + STM32 HAL 裸机工程，
使用协作式调度器而非 RTOS。

- MCU：STM32F103ZET6
- 开发环境：Keil MDK-ARM（`MDK-ARM/main.uvprojx`）
- 器件支持包：`Keil.STM32F1xx_DFP.2.4.1`
- 配置工具：STM32CubeMX（`main.ioc`）

## 功能概览

| 外设 | 用途 |
| --- | --- |
| USART1 (PA9/PA10) | USB-UART 调试控制台 |
| UART4 (PC10/PC11) | HMI 串口屏 |
| SPI2 | SPI Flash 读写 |
| ADC1 | VDDA、ADJ 电压 / 电流采样，MCU 内部温度 |
| IWDG | 独立看门狗 |
| GPIO | 指示灯、按键、蜂鸣器、SWITCH_EN、TF 卡检测 |
| 软件 I2C | EEPROM 参数存储 |
| 1-Wire | DS18B20 板载温度 |
| DWT | 微秒级延时与计时 |

## 软件结构

```
Core/                   CubeMX 生成：启动、时钟、外设初始化、中断
application/app/        应用层
    app_main.c          初始化、任务注册、主循环、看门狗喂狗
    app_sched.c         协作式周期任务调度器（最大 16 个任务）
    app_led.c           运行指示灯
    app_switch.c        SWITCH_EN 路由切换
    app_hmi.c           HMI 串口屏帧解析
    app_log.c           分级日志输出（USART1）
    app_board.c         板级监控：按键、蜂鸣器、TF 卡、ADC、DS18B20
application/bsp/        板级驱动层
    bsp_gpio / bsp_uart / bsp_adc / bsp_eeprom / bsp_soft_i2c
    bsp_spi_flash / bsp_ds18b20 / bsp_wdg / bsp_dwt
Drivers/                CMSIS + STM32F1xx HAL
MDK-ARM/                Keil 工程、启动文件、生成的 hex
```

## 任务调度

所有任务由 `app_sched` 以固定周期轮询调用，任务必须快速返回：

| 周期 | 任务 | 说明 |
| --- | --- | --- |
| 10 ms | `app_led_task` | 指示灯 |
| 10 ms | `app_switch_poll` | 切换命令检测 |
| 10 ms | `app_board_task_fast` | 按键、蜂鸣器状态机 |
| 10 ms | `bsp_uart_poll` | 串口收包分发 |
| 20 ms | `app_hmi_poll` | 屏帧解析 |
| 100 ms | `app_board_task_medium` | TF 卡检测、ADJ 电压 / 电流 |
| 500 ms | `app_board_task_slow` | VDDA、MCU 温度、DS18B20 转换 |

看门狗在**主循环**中喂狗，而不是在中断里，任务阻塞或跑飞时由 IWDG 复位整机。

## SWITCH_EN 路由

| SWITCH_EN | 连接 |
| --- | --- |
| 高（默认） | USB-UART ↔ MCU (USART1) |
| 低 | USB-UART ↔ HMI 屏，屏口改由 USART1 接入 |

该切换是单向的：切到屏之后 MCU 收不到 PC 的数据，因此应用任务全部停止，仅保留看门狗
（以及默认开启的 300 ms 心跳灯，可由 `APP_SWITCH_KEEP_LED` 关闭）。

## 编译

1. 用 Keil MDK 打开 `MDK-ARM/main.uvprojx`。
2. 编译目标 `main`，输出 `MDK-ARM/main/main.hex` 与 `main.axf`。

当前固件占用：

```
Code = 28196 B   RO-data = 544 B   RW-data = 88 B   ZI-data = 7360 B
```

## 烧录

- Keil + ST-Link：直接下载调试。
- 串口 ISP：运行 `MDK-ARM/main/0.Dowdload.py`（依赖 `pyserial` 与同目录下的
  `stm32flash.exe`），选择串口与 HEX 文件后按提示烧录，推荐波特率 460800。

## 备注

Keil 的中间产物（`.o`、`.d`、`.axf`、`.map`、`.htm`、`.lnp`）、`.uvoptx`、
`.uvguix.*` 以及本机 IDE 配置已在 `.gitignore` 中排除，仓库只保留源码与工程文件。