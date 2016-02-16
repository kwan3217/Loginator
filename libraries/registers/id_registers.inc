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
#define N_ID (BOOTSTRAP_BASE_ADDR-HW_ID_BASE_ADDR)/HW_ID_PERIPHERAL_BLOCK_SIZE
//The first describes the hardware type:
// 0 for Logomatic
// 1 for Loginator
// 2 for Rocketometer
ro0(id,HW_TYPE  ,HW_ID_BASE_ADDR+0x00);
#ifndef HW_TYPE_LOGOMATIC
#define HW_TYPE_LOGOMATIC     0
#define HW_TYPE_LOGINATOR     1
#define HW_TYPE_ROCKETOMETER  2
#define HW_TYPE_LOGINATOR2368 3
#endif
//The second word is a serial number, unique to hardware type. Therefore there
//can be a Loginator 0 and Rocketometer 0, but no two Rocketometers are
//both labeled 0.
ro0(id,HW_SERIAL,HW_ID_BASE_ADDR+0x04);
//Reports if the device is real or simulated. Should not be used to affect any
//program actions, just to be recorded in version information.
//0 - Not a simulator - IE real hardware.
//1 - Simulator
ro0(id,HW_SIM,   HW_ID_BASE_ADDR+0x08);

//Next comes information about peripherals
//Each block has a consistent format.
//Block offset  Description    
//------------  -------------  
// 0x00         part type      
// 0x04         port type
// 0x08         port number 
// 0x0C         address     
// 0x10         custom[0]
// 0x14         custom[1]
// 0x18         custom[2]
// 0x1C         custom[3]
// 0x20         Description
ro1(id,HW_ID_PART_TYPE,N_ID,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x00);
ro1(id,HW_ID_PORT_TYPE,N_ID,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x04);
ro1(id,HW_ID_PORT_NUM, N_ID,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x08);
ro1(id,HW_ID_ADDRESS,  N_ID,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x0C);
ro2(id,HW_ID_CUSTOM,   N_ID,4,HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+0x10+j*4);
ro1(id,HW_ID_DESC,     N_ID,  HW_ID_BASE_ADDR+i*HW_ID_PERIPHERAL_BLOCK_SIZE+HW_ID_PERIPHERAL_DESC);

//#undef HW_ID_BASE_ADDR
//#undef HW_ID_PERIPHERAL_BLOCK_SIZE
//#undef HW_ID_PERIPHERAL_DESC
