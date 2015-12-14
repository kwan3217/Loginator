ro0(vic,VICIRQStatus);
ro0(vic,VICFIQStatus);
ro0(vic,VICRawIntr);
rw0(vic,VICIntSelect);
rw0(vic,VICIntEnable);
wo0(vic,VICIntEnClr);
rw0(vic,VICSoftInt);
wo0(vic,VICSoftIntClr);
rw0(vic,VICProtection);
rw0(vic,VICVectAddr);
rw0(vic,VICDefVectAddr);
rw1(vic,VICVectAddrSlot);
rw1(vic,VICVectCntlSlot);


