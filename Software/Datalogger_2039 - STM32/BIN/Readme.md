Burn FLASH with St-Link and Android:

- This file (*.HEX) can be burned on the STM32F407VG using a smartphone Android with this ZFlasher App.

- - The smartphone needs to have USB OTG port.

- - Tested with St-Link v2.

https://play.google.com/store/apps/details?id=ru.zdevs.zflasherstm32&hl=en_US&gl=US

- Image rotation:
- - USB_HOST_HID_MSC_LCD_FLASH_RTC_ACC_TEMP3_Rotate.hex
- - - File: ILI9341.h
- - - From: #define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
- - - To: #define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
- - ILI9341 datasheet:
- - - 8.2.29. Memory Access Control (36h)
- - - 8.2.6. Read Display MADCTL (0Bh)

----------

Notes:
- Version 0.0.8 is very different from version 0.0.7, therefore version 0.0.7 was kept in the current repository.
- - Not all features of version 0.0.8 have been tested.
- - - Bin file version 0.0.8 in "Evaluation version" folder
-
- Warning: Back up data captured with firmware version 0.0.7 before upgrading to version 0.0.8.
- - The way of capturing and exporting has not been modified, only a bug has been fixed.
-
- The core USB support has been modified to support more than one HID interface.
- - In this way it can support the USB "Wireless Mini Keyboard with built-in Touchpad".
-
- The USB port requires constant scanning, so the microcontroller's processing capacity for updating the display is shared with the USB port.