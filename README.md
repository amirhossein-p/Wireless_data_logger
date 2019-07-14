# Wireless_data_logger
## Wireless temperature data logger
### Introduction
This is a Micro controller-based embedded system for the purpose of measuring temperature remotely. The system consists of two main parts, Data Logger and Transceiver. Data Logger’s duty is measuring temperature and sending a data string to the Transceiver. The transceiver is designed to receive the data string, decode it, display the values on an LCD, and store data on a Micro-SD card. The Transceiver is also able to communicate with a third-party device via Serial protocol. The task of wireless communication is carried out by two nRF modules and the temperature is measured with DALLAS Semiconductor’s DS1820 Thermometer.
### Usage
Temperature data loggers are widely used in industries such as food and agriculture. This system introduced particularly for the idea of real-time monitoring and controlling temperature inside a pasteurization machine, which is commonly used in beverage production lines.

To describe the pasteurization machine briefly, it is a long tunnel of about 20 meters in length and 3 to 5 meters in width. As beverage bottles pass through the tunnel slowly, a series of showers splash water with specified temperature on them. The temperature is varied in different stages and flexible to set by the operator. As a result, bottles’ temperature is regulated for specific time periods, so the desired pasteurization pattern will force to the bottles.

It is a common practice to put a temperature data logger into a random bottle in order to check the exact heat that the liquid inside the bottle received. For this purpose, the operator has to put the data logger in tunnel’s entrance and wait for about two hours to pick it at the exit of the tunnel.

By using the presented system, it is possible to access temperature data online. Consequently, this data can be either monitored by an operator or involved in the command process that set the temperature of each stage in the pasteurization machine. By doing so, we can achieve a real-time control system which receives feedback from the inside of the bottles, the final place to regulate temperature.

This project contains two devices:
* The Data Logger which measures temperature. It has an AVR micro controller, two Temperature sensors, an nRF module, and a Lithium-Polymer battery. It is built small enough to be entered into a conventional beverage bottle.
* The Transceiver which receives data. It has an AVR micro controller, an nRF module, a character LCD, a driver module for Micro-SD, an interface for serial communication, and a circuit for charging DL’s battery.
### Transceiver
#### Hardware
The main part of the TR is an Atmel’s ATMega16A AVR microcontroller. It is chosen since it covers all the needs of the project, it has enough ports and memory, and it is cheap and easy to use. Communication between the microcontroller and nRF module is delivered through SPI interface.  USART is used for connecting to the MicroSD module and the third-party device. A TTL buffer enables the access of these two devices to the single USART port of the microcontroller. A 4×16 character LCD is connected to digital pins of the microcontroller; it is used for displaying data and menus. There are 4 push buttons whose duty is to receive commands from the operator. As well as this, two LEDs show the function of receive and transmit. A simple power circuit based on regulator ICs generates two DC voltage levels required for different parts.
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Sch1.png">
</p>

Buffer prevents data conflict between the two devices which use the same USART port, by controlling the route chosen by Enable signal of the microcontroller. Therefore, the microcontroller selects the desired device by turning on the relating enable pin, and then it puts data on the USART port. This action happens repeatedly in each cycle.
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Sch2.png">
</p>

MAX232 is an RS232 driver and receiver. It presents communication between TR and other devices that have the RS232 interface. The designed frame sends all the data on LCD and receives four digital push-button signals. As a result, it is possible to completely control the TR over another device such as a PLC. As well as this, temperature data transfers to the third-party device. This function makes that device (especially a PLC) enable to control the temperature in a real-time process.

MicroSD module is a small circuit with an AVR. This module is built to ease the driving process of MicroSD. By using it, it is only needed to send pre-defined commands based on the module’s manual. In this project, a code library, especially for this module, made, so in the main code, only functions are called. The module also sends response codes and error numbers. It has two status outputs, one shows device busy and the other shows error. It is possible to reset the module through a digital signal.

The nRF module is a well-known 2.4GHz transceiver based on NORDIC Semiconductor’s nRF24L01+ chip. It has a lot of features, low power, high speed, multi-channel, all-in-one transmitter and receiver, and many more. Here it was chosen because of small size, appropriate voltage level, adequate distance coverage, and suitable interface. A special code library based on project needs is made to avoid getting involved with nRF configurations and commands in the middle of the main code.

Power circuit consists of simple regulator ICs, generating DC voltage levels of 5v and 3.3v. Battery charger circuit is a separate part included on TR’s PCB. A Microchip’s MCP73831 charge management controller undertakes the task of charging the Li-Polymer battery of DL.

#### Software
The most complicated part of the project is TR’s microcontroller code. It built-in C language by Atmel Studio. The technique of making header files and separating different parts of the code from each other significantly facilitated the task of code programming. Here are TR code’s main jobs:
* Communication with DL. TR has to transmit commands to DL and receive data from it through the nRF module. Data between these two is encoded into frames with a unique structure. By doing so, all transferring data is encapsulated into the minimum number of bytes. TR builds command frames and sends them to DL; moreover, it decodes the received frames from DL and prepares temperature values for further use.
* Interfacing with the operator. By means of a character LCD and four push-buttons, TR is able to show data and menus to the operator and receive instructions. There are a number of pages shown on LCD by which the operator can enter the setting, see values, and notice errors. Push-buttons are located in particular places above the LCD so that their function reads on the LCD and is varying in different pages.
* Storing values on MicroSD. This action takes place through the MicroSD module. The module is nothing but an AVR microcontroller with a driving circuit and a MicroSD slot. This module shoulders the burden of connecting to the memory and storing data on it. Thus, the main micro controller’s job reduces to just sending preferred commands through the USART port and receiving responses. Commands that are needed in the project has built and stored in a library in advance, so the main code only needs to call the header file and use the functions.  
* Interfacing with the third party device. The other TR’s duty is to connect to a PLC or any other device via RS232 protocol. Special frames are designed for this connection. Any device which has a serial port and able to both decode and encode frames can be RT’s third-party device. Originally a PLC has been considered as the third party device, however; any other system with mentioned specifications can be used. Frames are designed in a way that PLC (or other devices) can receive all the data that LCD illustrates, in addition; TR receives four bits as push buttons command. As a result, PLC is able to fully control the TR and also receive temperature data in real time.

TR code is also equipped with EEPROM storing functions that make way for keeping the operator’s settings after turning off and on. It also has counter systems which calculate time periods without pausing the whole process. Another feature is that microcontroller is able to reset modules in error situations; thus, the process of data logging will not totally stop but just interrupt for a short time.

### Data logger
#### Hardware
An Atmel’s ATMega8A AVR microcontroller plays the main role in DL’s hardware. The reasons for selecting this chip are low voltage level of 3.3v, enough I/O and memory, and simplicity of programming. DL commutes with TR through an nRF module which is connected to the microcontroller by SPI bus. Two temperature sensors are connected to the microcontroller by USART interface. Sensors are DALLAS Semiconductor’s DS1820 one-wire digital thermometers. They provide 9–bit temperature readings which indicate the temperature of the device. Information is sent to/from the DS1820 over a 1–Wire interface so that only one wire (and ground) needs to be connected from microcontroller to DS1820. Because each DS1820 contains a unique silicon serial number, multiple DS1820s can exist on the same 1–Wire bus []. They follow a complicated method for measuring temperature, and they calibrated by DALLAS Semiconductor in advance. DL also contains a circuit for measuring battery level by an analog to digital converter. The rest are basic circuits used to drive the microcontroller and other components. The most challenging factor in designing DL’s hardware was an effort for making it as small as possible. This is important since DL needs to be able to enter to a beverage bottle. For this purpose, the PCB designed two-sided, and small components chose.
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Sch3.png">
</p>

The battery is a 3.7v – 230mAh Lithium polymer. It is able to keep the circuit turned on easily for more than 4 hours, which is enough for the purpose of beverage pasteurization data logging. According to limited space in DL, the charging circuit is put in TR.

The housing for DL built out of stainless steel, with two caps sealed by o-rings. The above cap built from plastic in order that waves could pass. The other cap makes access to turn-on switch and charging plug. Needing get into a beverage bottle, the housing built as thin as possible. Therefore, as the inside has enough space for the circuit, the outer diameter is still smaller than a normal bottle’s. Conventional beverage bottles, which called “neck 28”, have the outer diameter of 28mm and the inner diameter of 22mm. the housing’s outer diameter is exactly 21mm.

#### Software
As well as TR, DL’s code is built in C language by Atmel Studio. Header files are used in the code, for making it easier. The header that drives nRF is same as TR, and there is a header which drives temperature sensors. On the whole, code written for DL is much simpler than TR’s. Here are parts of the code:
* USART and ADC configuration. The serial bus is used for making access to DS1820 sensors. It is set as sensor asks. ADC undertakes the task of measuring battery level. It reads an analog coming from battery level detection circuit.
* Active delay. This structure makes the controller pause between samples while it is able to receive the command from TR during the delay period. Duration of the delay, which is equal to the time period between samples, can be selected in TR’s setting page. This value is transferred with the command that turns DL on.
* Prepare and send data. Received data from two sensors, battery level, and a unique ID number are being sent to TR. These data are encoded and put in an especially-designed frame. Temperature values go through a mathematic process so that it can be divided into bytes without losing decimal value.
* Off acknowledgment. After receiving off command from TR, DL sends a sign showing the command is received. As TR receives this sign, it considers the process being stopped.

In fact, the main job is left for TR, and DL just transfers sensors’ data to TR.

### Handshaking
#### TR & DL
Data transfer between TR and DL consists of four steps. First, by operator’s instruction, TR sends an on command to DL. Then DL starts sending data to TR. These data frames are being sent continuously while DL is on. After that, by operator’s off instruction, TR sends off command to DL. As DL receives off command, it returns off acknowledgment sign to TR.
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Frm2.png">
</p>
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Frm1.png">
</p>


| Name          | Detail        |
|:-------------:|:-------------|
| fin           | This byte can be only 0 or 1. Turning finder on makes it 1, so DL turns finder LED on. Finder is a bright LED that can be used for discovering the data logger in a dark area. |
| dly      | This byte contains a number which indicates time intervals between samples.      |
| r11 – r13 | These bytes contain the temperature value of sensor 1. Sign and digit are included in the first byte, the second byte contains MSB part of decimal and the third one is used for LSB part of decimal.      |
| bat | Battery level byte can be a number from 0 to 8.       |
| r21 – r23 | Temperature sensor 2 value. Data is ordered in these three bytes as well as data for sensor 1.      |
| id | A unique number that introduces DL. This is particularly useful when more than one data logger is in service at a time.      |

Each frame contains a number of fixed characters which are used to detect the frame and to check that correct length is received. Working as a receiver, both TR and DL check the characters at the start and end of the frame. For instance, regarding TR, if the character ‘A’ is received at the position of the first byte and character ‘B’ at 10th, TR indicates that a complete Data frame has received. Then, it starts to decode data.

#### TR & Third-party device
The third-party device, as described, is an external device with RS232 protocol that can fully control the TR and have access to all data showed on LCD. Since the USART port is shared between the third-party device and the MicroSD module, there are hardware and software principles that make them able to work simultaneously. As described, the hardware part is a buffer that enables the microcontroller to select the device connected to the port. The software puts each data and activates enable pins synchronously and repeatedly. Normally enable pins are activated in a way that the third-party device could communicate with the microcontroller. TR sends a specific frame containing LCD’s data to the third-party device on each cycle, and the third-party device should send a particular frame containing the state of push buttons to TR on fixed intervals. Meanwhile, whenever communication with the MicroSD module is on-demand, enable pins will change their state and data related to the MicroSD module will be prepared on the port. Details about transferring frames between TR and third-party device are illustrated in figure 8.
<p align="center">
<img src="https://raw.githubusercontent.com/amirhossein-p/Wireless_data_logger/master/media/Frm3.PNG">
</p>

As shown above, transmitting the frame from TR to the third-party device contains 18 bytes. Fixed characters at start and end define the frame’s length. Encoded bytes are drawn separately. Frame sent from the third-party device to TR is only one byte. Allocated names to each byte are the same as variables used in the code.

| Name          | Detail        |
|:-------------:|:-------------|
| sgn1           | Contains sign of temperature 1 value. 1 stand for minus.  |
| dig1           | Digit part of temperature 1 value. |
| r12 , r13           | MSB and LSB bytes for decimal part of temperature 1 value. |
| sgn2           | Contains sign of temperature 2 value. 1 stand for minus. |
| dig2           | Digit part of temperature 2 value. |
| r22 , r23           | MSB and LSB bytes for decimal part of temperature 2 value. |
| disp1           | Encoded byte containing LCD signs and numbers. |
| disp2           | Encoded byte containing LCD signs and numbers. |
| fnc           | MicroSD current file number. |
| sdrp           | MicroSD current error number. |
| dsc2           | Encoded byte containing LCD signs and dsc MSB byte. |
| dsc1           | LSB byte for dsc. |
| rsid           | Resend indicator bit. 1 means that the sign is shown on LCD.  |
| fin           | Finder indicator bit. |
| onid           | DL running indicator bit. |
| ofak           | DL stopped indicator bit. |
| pg           | LCD current page number. |
| tots           | TR current error number. |
| dsc           | MicroSD data string counter. Shows the sample number stored on MicroSD. |
| errs           | MicroSD error indicator bit. This bit functions as an error LED mounted on TR’s box.  |
| mems           | MicroSD working indicator bit. |
| usrv           | Receiving byte from the third-party device that tells which key is pressed by the third-party device. |

By receiving these data from TR and knowing frames principle, it is possible to simulate TR’s panel on a third-party device. Furthermore, received temperature data can be assumed as feedback for real-time temperature control process.
