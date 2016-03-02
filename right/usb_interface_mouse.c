#include "include/board/board.h"
#include "fsl_gpio.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "include/usb/usb_device_class.h"
#include "include/usb/usb_device_hid.h"
#include "include/usb/usb_device_ch9.h"
#include "composite.h"
#include "usb_class_mouse.h"
#include "usb_interface_mouse.h"
#include "usb_descriptor_configuration.h"

static usb_mouse_report_t UsbMouseReport;

static uint8_t scrollCounter = 0;
static volatile usb_status_t UsbMouseAction(void)
{
    UsbMouseReport.buttons = 0;
    UsbMouseReport.x = 0;
    UsbMouseReport.y = 0;
    UsbMouseReport.wheelX = 0;
    UsbMouseReport.wheelY = 0;

    if (!GPIO_ReadPinInput(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN)) {
        if (!(scrollCounter % 10)) {
            UsbMouseReport.wheelX = -1;
        }
    }
    scrollCounter++;
    return USB_DeviceHidSend(UsbCompositeDevice.mouseHandle, USB_MOUSE_ENDPOINT_ID,
                             (uint8_t*)&UsbMouseReport, USB_MOUSE_REPORT_LENGTH);
}

usb_status_t UsbMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event) {
        case kUSB_DeviceHidEventSendResponse:
            if (UsbCompositeDevice.attach) {
                return UsbMouseAction();
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

usb_status_t UsbMouseSetConfiguration(class_handle_t handle, uint8_t configuration)
{
    if (USB_COMPOSITE_CONFIGURATION_INDEX == configuration) {
        return UsbMouseAction();
    }
    return kStatus_USB_Error;
}

usb_status_t UsbMouseSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_MOUSE_INTERFACE_INDEX == interface) {
        return UsbMouseAction();
    }
    return kStatus_USB_Error;
}