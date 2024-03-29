- Support for [TMP117](https://www.ti.com/product/TMP117) (+/-0.1°C; 16 bits) sensors (8CH using I2C Multiplexer IC [TCA9548A](https://www.sparkfun.com/products/16784) connected on the I2C3 data bus, connection [reference U17](https://github.com/rtek1000/Datalogger_2039/blob/main/Hardware/Datalogger_2039.pdf) (24AA02E48-I/SN not placed))
- - TMP117 high-accuracy temperature sensor
- - - ±0.1 °C (maximum) from –20 °C to +50 °C
- - - ±0.15 °C (maximum) from –40 °C to +70 °C
- - - ±0.2 °C (maximum) from –40 °C to +100 °C
- - - ±0.25 °C (maximum) from –55 °C to +125 °C
- - - ±0.3 °C (maximum) from –55 °C to +150 °C
- - - Operating temperature range: –55 °C to +150 °C (For model with letter M: 0°C to 85°C only)
- - Due to the high sensitivity of the sensor, manual soldering can affect the TMP117, it may be more advisable to buy assembled PCB boards, for example [Adafruit](https://www.adafruit.com/product/4821) and [SparkFun](https://www.sparkfun.com/products/15805) boards
- - - WSON package: [SON Temperature Profile](https://www.ti.com/lit/an/slua271b/slua271b.pdf)
- - - - Rework Guidelines: "Do not reuse the part after it is removed" (Pag 16, Table 3)
-
- - - DSBGA package: [BGA Temperature Profile](https://www.ti.com/lit/wp/ssyz015b/ssyz015b.pdf)
-
- - - SON package ([Infineon](https://www.infineon.com/dgdl/Infineon-Additional_product_information_SON_packages-AN-v00_01-EN.pdf?fileId=db3a30433e82b1cf013e82faab2000e5)): "Due to possible damage while removing the component, a desoldered component
should not be reused"
-
- - Maybe the 138°C soldering (Sn/Bi) could be used to prevent damage to the sensor and perhaps allow reuse after the sensor is removed from the board (possibly applicable for model with letter M, TMP117M, working temperature 0°C to 85°C only).
 
- TMP117 Custom Board:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/TMP117/TMP117_Custom_Board/TMP117_custom_board/TMP117_custom_board_Top2.png)

- TMP117 Custom Board schematic:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/TMP117/TMP117_Custom_Board/TMP117_custom_board/TMP117_custom_board_Schematic.png)

- Interface adapter:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/TMP117/Datalogger2039_TMP117_adapter/Datalogger2039_TMP117_adapter_1.png)

- Interface schematic:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/TMP117/Datalogger2039_TMP117_adapter/Datalogger2039_TMP117_adapter_Main_Schematic.png)
