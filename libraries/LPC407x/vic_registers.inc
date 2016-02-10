#define VIC_BASE_ADDR 0xFFFFF000

ro0(vic,VICIRQStatus  ,VIC_BASE_ADDR+0x000);
ro0(vic,VICFIQStatus  ,VIC_BASE_ADDR+0x004);
ro0(vic,VICRawIntr    ,VIC_BASE_ADDR+0x008);
rw0(vic,VICIntSelect  ,VIC_BASE_ADDR+0x00C);
rw0(vic,VICIntEnable  ,VIC_BASE_ADDR+0x010);
wo0(vic,VICIntEnClr   ,VIC_BASE_ADDR+0x014);
rw0(vic,VICSoftInt    ,VIC_BASE_ADDR+0x018);
wo0(vic,VICSoftIntClr ,VIC_BASE_ADDR+0x01C);
rw0(vic,VICProtection ,VIC_BASE_ADDR+0x020);
rw0(vic,VICVectAddr   ,VIC_BASE_ADDR+0x030);
rw0(vic,VICDefVectAddr,VIC_BASE_ADDR+0x034);
rw1(vic,VICVectAddrSlot,16,VIC_BASE_ADDR+0x100+0x04*i);
rw1(vic,VICVectCntlSlot,16,VIC_BASE_ADDR+0x200+0x04*i);

#undef VIC_BASE_ADDR
