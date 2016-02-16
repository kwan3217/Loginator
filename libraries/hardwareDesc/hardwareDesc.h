#ifndef hardwareDesc_h
#define hardwareDesc_h

#define HARDWARE_DESC

//Next comes information about peripherals
//Each block has a consistent format.
//Block offset  Description    value   Value description
//------------  -------------  -----   -----------------
// 0x00         part type

enum class partType:unsigned int {
  sdCard,   //                 0       SD card
  mpu60x0,  //                 1       MPU60x0/9150 (just inertial part)
  l3g4200d, //                 2       L3G4200D 3 axis rotation sensor
  ad799x,   //                 3       AD799x ADC
  adxl345,  //                 4       ADXL345 3 axis accelerometer
  ak8975,   //                 5       AK8975 3 axis magnetometer
            //                           (used in MPU9150)
  bmp180,   //                 6       BMP085/180 pressure sensor
  hmc5883,  //                 7       HMC5883L 3 axis magnetometer
  led,      //                 8       LED
  bma180,   //                 9       BMA180 3 axis accelerometer
  nmea,     //                 10(0xA) NMEA GPS
  button,   //                 11(0xB) Button
  servo,    //                 12(0xC) Servo control
  uart,     //                 13(0xD) Universal Async Receiver/Transmitter
  unknown=0xFFFF'FFFF };//     unk     No part in this or any further slot
//Block offset  Description    value   Value description
//------------  -------------  -----   -----------------
// 0x04         port type
enum class portType {
  SPI,
  I2C,
  GPIO,
  PWM,
  UART};
// 0x08         port number    GPIO    Py.xx y number
//                             PWM     channel number
//                             I2C     port number
// 0x0C         address        SPI     P0.xx pin used for CS for SPI parts
//                             I2C     I2C 7-bit address for I2C parts
//                             GPIO    Py.xx pin used for this part
//                                       (y is port number from above, so P1.xx
//                                        can be used).
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
//Interrupts - available on GPS (PPS), ADXL345, L3G4200D, HMC5883, Button
//If custom[0] is unknown, the part doesn't have the interrupt line connected
//Block offset      Description    value   Value description
//------------      -------------  -----   -----------------
// 0x10 (custom[0]) Capture port             CAPy.x y value for this interrupt
// 0x14 (custom[1]) Capture channel          CAPy.x x value for this interrupt
// 0x18 (custom[2]) Pin number               bits 4:0 - P0.xx x value
//                                           bits 6:5 - value for PINSELx
// 0x1C (custom[3]) edge ID                  used to identify rising, falling,
//                                             or both edges. These bits are
//                                             shifted the right amount to feed TCCR(y)
//
//Servo control - Each servo has a separate custom[0] number. Exact interpretation
//depends on the robot in question, but for instance on Yukari we have:
//Block offset      Description    value   Value description
//------------      -------------  -----   -----------------
// 0x10 (custom[0]) color          0       steering
//                                 1       throttle
//UART
//Describes which UART is connected to which physical pins, and what PINSEL to use
//for each pin.
//Block offset      Description    value   Value description
//------------      -------------  -----   -----------------
// 0x8 (port)                              UARTx x number for this port, IE
//                                         if you are setting up UART2, look for
//                                         a 2 in this slot. Port type and address
//                                         are unused for this device type
// 0x18 (custom[0]) Rx Pin selection       bits  0: 7 - Py.xx x value
//                                         bits 15: 8 - Py.xx y value
//                                         bits 31:16 - PINSEL value
// 0x1C (custom[1]) Tx Pin selection       bits  0: 7 - Py.xx x value
//                                         bits 15: 8 - Py.xx y value
//                                         bits 31:16 - PINSEL value
#endif
