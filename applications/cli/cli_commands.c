#include "cli_commands.h"
#include <furi-hal.h>
#include <furi-hal-gpio.h>
#include <furi-hal-info.h>
#include <rtc.h>
#include <task-control-block.h>
#include <time.h>
#include <notification/notification-messages.h>

void cli_command_device_info_callback(const char* key, const char* value, bool last, void* context) {
    printf("%-24s: %s\r\n", key, value);
}

/* 
 * Device Info Command
 * This command is intended to be used by humans
 */
void cli_command_device_info(Cli* cli, string_t args, void* context) {
    furi_hal_info_get(cli_command_device_info_callback, context);
}

void cli_command_help(Cli* cli, string_t args, void* context) {
    (void)args;
    printf("Commands we have:");

    // Command count
    const size_t commands_count = CliCommandTree_size(cli->commands);
    const size_t commands_count_mid = commands_count / 2 + commands_count % 2;

    // Use 2 iterators from start and middle to show 2 columns
    CliCommandTree_it_t it_left;
    CliCommandTree_it(it_left, cli->commands);
    CliCommandTree_it_t it_right;
    CliCommandTree_it(it_right, cli->commands);
    for(size_t i = 0; i < commands_count_mid; i++) CliCommandTree_next(it_right);

    // Iterate throw tree
    for(size_t i = 0; i < commands_count_mid; i++) {
        printf("\r\n");
        // Left Column
        if(!CliCommandTree_end_p(it_left)) {
            printf("%-30s", string_get_cstr(*CliCommandTree_ref(it_left)->key_ptr));
            CliCommandTree_next(it_left);
        }
        // Right Column
        if(!CliCommandTree_end_p(it_right)) {
            printf("%s", string_get_cstr(*CliCommandTree_ref(it_right)->key_ptr));
            CliCommandTree_next(it_right);
        }
    };

    if(string_size(args) > 0) {
        cli_nl();
        printf("Also I have no clue what '");
        printf("%s", string_get_cstr(args));
        printf("' is.");
    }
}

void cli_command_date(Cli* cli, string_t args, void* context) {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    if(string_size(args) > 0) {
        uint16_t Hours, Minutes, Seconds, Month, Date, Year, WeekDay;
        int ret = sscanf(
            string_get_cstr(args),
            "%hu:%hu:%hu %hu-%hu-%hu %hu",
            &Hours,
            &Minutes,
            &Seconds,
            &Month,
            &Date,
            &Year,
            &WeekDay);
        if(ret == 7) {
            time.Hours = Hours;
            time.Minutes = Minutes;
            time.Seconds = Seconds;
            time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
            time.StoreOperation = RTC_STOREOPERATION_RESET;
            date.WeekDay = WeekDay;
            date.Month = Month;
            date.Date = Date;
            date.Year = Year - 2000;
            HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
            HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);

            // Verification
            HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
            printf(
                "New time is: %.2d:%.2d:%.2d %.2d-%.2d-%.2d %d",
                time.Hours,
                time.Minutes,
                time.Seconds,
                date.Month,
                date.Date,
                2000 + date.Year,
                date.WeekDay);
        } else {
            printf(
                "Invalid time format, use `hh:mm:ss MM-DD-YYYY WD`. sscanf %d %s",
                ret,
                string_get_cstr(args));
            return;
        }
    } else {
        // TODO add get_datetime to core, not use HAL here
        // READ ORDER MATTERS! Time then date.
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
        printf(
            "%.2d:%.2d:%.2d %.2d-%.2d-%.2d %d",
            time.Hours,
            time.Minutes,
            time.Seconds,
            date.Month,
            date.Date,
            2000 + date.Year,
            date.WeekDay);
    }
}

void cli_command_log(Cli* cli, string_t args, void* context) {
    furi_stdglue_set_global_stdout_callback(cli_stdout_callback);
    printf("Press any key to stop...\r\n");
    cli_getc(cli);
    furi_stdglue_set_global_stdout_callback(NULL);
}

void cli_command_vibro(Cli* cli, string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        NotificationApp* notification = furi_record_open("notification");
        notification_message_block(notification, &sequence_reset_vibro);
        furi_record_close("notification");
    } else if(!string_cmp(args, "1")) {
        NotificationApp* notification = furi_record_open("notification");
        notification_message_block(notification, &sequence_set_vibro_on);
        furi_record_close("notification");
    } else {
        cli_print_usage("vibro", "<1|0>", string_get_cstr(args));
    }
}

void cli_command_led(Cli* cli, string_t args, void* context) {
    // Get first word as light name
    NotificationMessage notification_led_message;
    string_t light_name;
    string_init(light_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        string_clear(light_name);
        return;
    } else {
        string_set_n(light_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Check light name
    if(!string_cmp(light_name, "r")) {
        notification_led_message.type = NotificationMessageTypeLedRed;
    } else if(!string_cmp(light_name, "g")) {
        notification_led_message.type = NotificationMessageTypeLedGreen;
    } else if(!string_cmp(light_name, "b")) {
        notification_led_message.type = NotificationMessageTypeLedBlue;
    } else if(!string_cmp(light_name, "bl")) {
        notification_led_message.type = NotificationMessageTypeLedDisplay;
    } else {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        string_clear(light_name);
        return;
    }
    string_clear(light_name);
    // Read light value from the rest of the string
    char* end_ptr;
    uint32_t value = strtoul(string_get_cstr(args), &end_ptr, 0);
    if(!(value < 256 && *end_ptr == '\0')) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        return;
    }

    // Set led value
    notification_led_message.data.led.value = value;

    // Form notification sequence
    const NotificationSequence notification_sequence = {
        &notification_led_message,
        NULL,
    };

    // Send notification
    NotificationApp* notification = furi_record_open("notification");
    notification_internal_message_block(notification, &notification_sequence);
    furi_record_close("notification");
}

void cli_command_gpio_set(Cli* cli, string_t args, void* context) {
    char pin_names[][4] = {
        "PC0",
        "PC1",
        "PC3",
        "PB2",
        "PB3",
        "PA4",
        "PA6",
        "PA7",
#ifdef FURI_DEBUG
        "PA0",
        "PB7",
        "PB8",
        "PB9"
#endif
    };
    GpioPin gpio[] = {
        {.port = GPIOC, .pin = LL_GPIO_PIN_0},
        {.port = GPIOC, .pin = LL_GPIO_PIN_1},
        {.port = GPIOC, .pin = LL_GPIO_PIN_3},
        {.port = GPIOB, .pin = LL_GPIO_PIN_2},
        {.port = GPIOB, .pin = LL_GPIO_PIN_3},
        {.port = GPIOA, .pin = LL_GPIO_PIN_4},
        {.port = GPIOA, .pin = LL_GPIO_PIN_6},
        {.port = GPIOA, .pin = LL_GPIO_PIN_7},
#ifdef FURI_DEBUG
        {.port = GPIOA, .pin = LL_GPIO_PIN_0}, // IR_RX (PA0)
        {.port = GPIOB, .pin = LL_GPIO_PIN_7}, // UART RX (PB7)
        {.port = GPIOB, .pin = LL_GPIO_PIN_8}, // SPEAKER (PB8)
        {.port = GPIOB, .pin = LL_GPIO_PIN_9}, // IR_TX (PB9)
#endif
    };
    uint8_t num = 0;
    bool pin_found = false;

    // Get first word as pin name
    string_t pin_name;
    string_init(pin_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        cli_print_usage("gpio_set", "<pin_name> <0|1>", string_get_cstr(args));
        string_clear(pin_name);
        return;
    } else {
        string_set_n(pin_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Search correct pin name
    for(num = 0; num < sizeof(pin_names) / sizeof(char*); num++) {
        if(!string_cmp(pin_name, pin_names[num])) {
            pin_found = true;
            break;
        }
    }
    if(!pin_found) {
        printf("Wrong pin name. Available pins: ");
        for(uint8_t i = 0; i < sizeof(pin_names) / sizeof(char*); i++) {
            printf("%s ", pin_names[i]);
        }
        string_clear(pin_name);
        return;
    }
    string_clear(pin_name);
    // Read "0" or "1" as second argument to set or reset pin
    if(!string_cmp(args, "0")) {
        LL_GPIO_SetPinMode(gpio[num].port, gpio[num].pin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(gpio[num].port, gpio[num].pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_ResetOutputPin(gpio[num].port, gpio[num].pin);
    } else if(!string_cmp(args, "1")) {
#ifdef FURI_DEBUG
        if(num == 8) { // PA0
            printf(
                "Setting PA0 pin HIGH with TSOP connected can damage IR receiver. Are you sure you want to continue? (y/n)?\r\n");
            char c = cli_getc(cli);
            if(c != 'y' && c != 'Y') {
                printf("Cancelled.\r\n");
                return;
            }
        }
#endif

        LL_GPIO_SetPinMode(gpio[num].port, gpio[num].pin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(gpio[num].port, gpio[num].pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetOutputPin(gpio[num].port, gpio[num].pin);
    } else {
        printf("Wrong 2nd argument. Use \"1\" to set, \"0\" to reset");
    }
    return;
}

void cli_command_ps(Cli* cli, string_t args, void* context) {
    const uint8_t threads_num_max = 32;
    osThreadId_t threads_id[threads_num_max];
    uint8_t thread_num = osThreadEnumerate(threads_id, threads_num_max);
    printf(
        "%-20s %-14s %-8s %-8s %s\r\n", "Name", "Stack start", "Heap", "Stack", "Stack min free");
    for(uint8_t i = 0; i < thread_num; i++) {
        TaskControlBlock* tcb = (TaskControlBlock*)threads_id[i];
        printf(
            "%-20s 0x%-12lx %-8d %-8ld %-8ld\r\n",
            osThreadGetName(threads_id[i]),
            (uint32_t)tcb->pxStack,
            memmgr_heap_get_thread_memory(threads_id[i]),
            (uint32_t)(tcb->pxEndOfStack - tcb->pxStack + 1) * sizeof(StackType_t),
            osThreadGetStackSpace(threads_id[i]));
    }
    printf("\r\nTotal: %d", thread_num);
}

void cli_command_free(Cli* cli, string_t args, void* context) {
    printf("Free heap size: %d\r\n", memmgr_get_free_heap());
    printf("Minimum heap size: %d\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %d\r\n", memmgr_heap_get_max_free_block());
}

void cli_command_free_blocks(Cli* cli, string_t args, void* context) {
    memmgr_heap_printf_free_blocks();
}

void cli_command_i2c(Cli* cli, string_t args, void* context) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    uint8_t test = 0;
    printf("Scanning external i2c on PC0(SCL)/PC1(SDA)\r\n"
           "Clock: 100khz, 7bit address\r\n"
           "\r\n");
    printf("  | 0 1 2 3 4 5 6 7 8 9 A B C D E F\r\n");
    printf("--+--------------------------------\r\n");
    for(uint8_t row = 0; row < 0x8; row++) {
        printf("%x | ", row);
        for(uint8_t column = 0; column <= 0xF; column++) {
            bool ret = furi_hal_i2c_rx(
                &furi_hal_i2c_handle_external, ((row << 4) + column) << 1, &test, 1, 2);
            printf("%c ", ret ? '#' : '-');
        }
        printf("\r\n");
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "!", CliCommandFlagParallelSafe, cli_command_device_info, NULL);
    cli_add_command(cli, "device_info", CliCommandFlagParallelSafe, cli_command_device_info, NULL);

    cli_add_command(cli, "?", CliCommandFlagParallelSafe, cli_command_help, NULL);
    cli_add_command(cli, "help", CliCommandFlagParallelSafe, cli_command_help, NULL);

    cli_add_command(cli, "date", CliCommandFlagParallelSafe, cli_command_date, NULL);
    cli_add_command(cli, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_add_command(cli, "ps", CliCommandFlagParallelSafe, cli_command_ps, NULL);
    cli_add_command(cli, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_add_command(cli, "free_blocks", CliCommandFlagParallelSafe, cli_command_free_blocks, NULL);

    cli_add_command(cli, "vibro", CliCommandFlagDefault, cli_command_vibro, NULL);
    cli_add_command(cli, "led", CliCommandFlagDefault, cli_command_led, NULL);
    cli_add_command(cli, "gpio_set", CliCommandFlagDefault, cli_command_gpio_set, NULL);
    cli_add_command(cli, "i2c", CliCommandFlagDefault, cli_command_i2c, NULL);
}
