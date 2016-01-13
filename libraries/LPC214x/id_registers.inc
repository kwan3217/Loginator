/* Hardware ID */
//We write hardware identifiers here, in its own memory page. The chosen page is
//16kiB below the top of LPC2148 flash. The top 12kiB (0x7D000-0x7FFFF) is IAP,
//so there is a total of 4kiB available here. In all cases, undefined values are
//reserved for future expansion, and 0xFFFF'FFFF means that the value is
//unknown (IE the value was never written to).
//0x7C000-0x7CFFF is a single sector (26, 0x1A), so this data can be written
//without disturbing the rest of memory, and vice versa.

#define HW_ID_BASE_ADDR 0x0007'C000
#define BOOTSTRAP_BASE_ADDR 0x0007'D000
#define HW_ID_PERIPHERAL_BLOCK_SIZE 0x40
#define HW_ID_PERIPHERAL_DESC HW_ID_PERIPHERAL_BLOCK_SIZE/2
#define N (BOOTSTRAP_BASE_ADDR-HW_ID_BASE_ADDR)/HW_ID_PERIPHERAL_BLOCK_SIZE
//The first describes the hardware type:
// 0 for Logomatic
// 1 for Loginator
// 2 for Rocketometer
ro0(id,HW_TYPE  ,HW_ID_BASE_ADDR+0x00);
#ifndef HW_TYPE_LOGOMATIC
#define HW_TYPE_LOGOMATIC    0
#define HW_TYPE_LOGINATOR    1
#define HW_TYPE_ROCKETOMETER 2
#endif
//The second word is a serial number, unique to hardware type. Therefore there
//can be a Loginator 0 and Rocketometer 0, but no two Rocketometers are
//both labeled 0. Again, 0xFFFFFFFF is unknown.
ro0(id,HW_SERIAL,HW_ID_BASE_ADDR+0x04);
//Reports if the device is real or simulated. Should not be used to affect any
//program actions, just to be recorded in version information.
//0 - Not a simulator - IE real hardware.
//1 - Simulator
//0xFFFF'FFFF - unknown (memory block never written to)
ro0(id,HW_SIM,   HW_ID_BASE_ADDR+0x08);

//Next comes information about peripherals
//Each block has a consistent format.
//Block offset  Description    value   Value description
//------------  -------------  -----   -----------------
// 0x00         part type      0       SD card
//                             1       MPU60x0/9150 (just inertial part)
//                             2       L3G4200D 3 axis rotation sensor
//                             3       AD799x ADC
//                             4       ADXL345 3 axis accelerometer
//                             5       AK8975 3 axis magnetometer
//                                       (used in MPU9150)
//                             6       BMP085/180 pressure sensor
//                             7       HMC5883L 3 axis magnetometer
//                             8       LED
//                             9       BMA180 3 axis accelerometer
//                             10(0xA) NMEA GPS
//                             11(0xB) Button/external input
//                             12(0xC) Servo control
//                             unk     No part in this or any further slot
// 0x04         port type      0       SPI
//                             1       I2C
//                             2       GPIO
//                             3       PWM
//                             4       Timer
//                             5       UART
// 0x08         port number
// 0x0C         address        SPI     P0.xx pin used for CS for SPI parts
//                             I2C     I2C 7-bit address for I2C parts
//                             GPIO    Py.xx pin used for this part
//                                       (y is port number from above, so P1.xx
//                                        can be used).
//                             PWM/timer P0.xx used for this device, must be one
//                                        of the pins which support this PWM
//                                        channel
// 0x20         Description            C string (null-terminated) human-readable
//                                     description of the part
//
//In principle the same type of device may appear multiple times, like for the
//tree rocketometer. Block 0 overlaps the device ID, but cannot be used as
//the device ID has a completely different form, and since we have precious data
//in that form already, we have to keep that form.
//The remainder of the space is four words which have an interpretation which
//depends on the part.
//LED
//Block offset      Description    value   Value description
//------------      -------------  -----   -----------------
// 0x10 (custom[0]) color          0       red
//                                 1       green
//                                 2       blue
//                                 3       amber
//
//Timer Capture
//Block offset      Description    value   Value description
//------------      -------------  -----   -----------------
// 0x10 (custom[0]) Capture channel         CAPy.x x value for this device, y
//                                            is channel number above
ro1(id,HW_ID_PART_TYPE,N,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x00);
ro1(id,HW_ID_BUS_TYPE, N,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x04);
ro1(id,HW_ID_PORT,     N,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x08);
ro1(id,HW_ID_ADDRESS,  N,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x0C);
ro2(id,HW_ID_CUSTOM,   N,3,HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x10+j*4);
ro2(id,HW_ID_DESC,     N,8,HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+HW_ID_PERIPHERAL_DESC+j*4);

#undef HW_ID_BASE_ADDR
#undef HW_ID_PERIPHERAL_BLOCK_SIZE
#undef HW_ID_PERIPHERAL_DESC
#undef N
