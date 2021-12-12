#pragma once

typedef enum {
    GpioStartEventUsbUart = 0,

    GpioUsbUartEventConfig,
    GpioUsbUartEventEnOff,
    GpioUsbUartEventEnOn,
    GpioUsbUartEventCntReset,
} GpioCustomEvent;
