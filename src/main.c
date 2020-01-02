#include <X11/Xlib.h>
#include "NVCtrlLib.h"

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

static volatile bool keep_running = true;

void ctrl_c_handler (int dummy)
{
    keep_running = false;
}

typedef struct {
    int temp_turn_on;
    int temp_turn_off;
    int (*calc_fan_speed)(int);
    int check_time_seconds;
} FanConfig;

int
get_fan_speed (int temperature)
{
    return temperature >= 100 ? 100 :
           temperature <=  60 ? 60  : temperature;
}

FanConfig default_fan_config = { 56, 50, get_fan_speed, 3 };

typedef struct {
    int number_of_coolers;
    int indices[127];
} CoolerInfo;

bool
get_cooler_info (Display *dpy, CoolerInfo* out_cooler_info)
{
    int len;
    unsigned char* ptr;

    int ret = XNVCTRLQueryTargetBinaryData (
        dpy,
        NV_CTRL_TARGET_TYPE_GPU,
        0, // target_id
        0, // display_mask
        NV_CTRL_BINARY_DATA_COOLERS_USED_BY_GPU,
        &ptr,
        &len
    );

    if (!ret)
        return false;

    int number_of_coolers = ((int*)ptr)[0];
    out_cooler_info->number_of_coolers = number_of_coolers;

    int* cooler_indices = &((int*)ptr)[1];
    for (int i = 0; i < number_of_coolers; i++)
        out_cooler_info->indices[i] = cooler_indices[i];

    XFree (ptr);
    return true;
}

int
main ()
{
    signal(SIGINT, ctrl_c_handler);

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(NULL));
        return 1;
    }

    int fan_controllable = XNVCTRLSetTargetAttributeAndGetStatus (
        dpy,
        NV_CTRL_TARGET_TYPE_GPU,
        0, // target_id
        0, // display_mask
        NV_CTRL_GPU_COOLER_MANUAL_CONTROL,
        1);

    if (!fan_controllable)
    {
        fprintf(stderr, "Cannot fan controllable.\n");
        return 1;
    }

    CoolerInfo cooler_info;
    if (!get_cooler_info (dpy, &cooler_info))
    {
        fprintf(stderr, "Cannot get cooler info.\n");
        return 1;
    };

    int temperature = 0;
    bool fan_spinning = true;
    XNVCTRLQueryTargetAttribute (
        dpy,
        NV_CTRL_TARGET_TYPE_THERMAL_SENSOR,
        0, // target_id
        0, // display_mask
        NV_CTRL_THERMAL_SENSOR_READING,
        &temperature
    );

    if (temperature <= default_fan_config.temp_turn_on)
        for (int c = 0; c < cooler_info.number_of_coolers; c++)
            XNVCTRLSetTargetAttributeAndGetStatus (
                dpy,
                NV_CTRL_TARGET_TYPE_COOLER,
                cooler_info.indices[c],
                0, // display_mask
                NV_CTRL_THERMAL_COOLER_LEVEL,
                default_fan_config.calc_fan_speed (temperature)
            );

    while (keep_running)
    {
        XNVCTRLQueryTargetAttribute (
            dpy,
            NV_CTRL_TARGET_TYPE_THERMAL_SENSOR,
            0, // target_id
            0, // display_mask
            NV_CTRL_THERMAL_SENSOR_READING,
            &temperature
        );

        if (fan_spinning && temperature <= default_fan_config.temp_turn_off)
        {
            fan_spinning = false;
            for (int c = 0; c < cooler_info.number_of_coolers; c++)
                XNVCTRLSetTargetAttributeAndGetStatus (
                    dpy,
                    NV_CTRL_TARGET_TYPE_COOLER,
                    cooler_info.indices[c],
                    0, // display_mask
                    NV_CTRL_THERMAL_COOLER_LEVEL,
                    0  // fan speed
                );
        }
        if (temperature >= default_fan_config.temp_turn_on)
        {
            fan_spinning = true;
            int fan_speed = default_fan_config.calc_fan_speed (temperature);
            for (int c = 0; c < cooler_info.number_of_coolers; c++)
                XNVCTRLSetTargetAttributeAndGetStatus (
                    dpy,
                    NV_CTRL_TARGET_TYPE_COOLER,
                    cooler_info.indices[c],
                    0, // display_mask
                    NV_CTRL_THERMAL_COOLER_LEVEL,
                    fan_speed
                );
        }

        sleep(default_fan_config.check_time_seconds);
    }
    printf ("\nSetting Cooler Manual Control\n");
    XNVCTRLSetTargetAttributeAndGetStatus (
        dpy,
        NV_CTRL_TARGET_TYPE_GPU,
        0, // target_id
        0, // display_mask
        NV_CTRL_GPU_COOLER_MANUAL_CONTROL,
        0
    );

    return 0;
}
