/* USB Controller */
#define USB_BASE_ADDR 0xE0090000
/* Device Interrupt Registers */
//USBIntSt (rw, addr 0xE01FC1C0,=base+0x16C1C0) not defined
ro0(usb,USBDevIntSt ,USB_BASE_ADDR+0x00);
rw0(usb,USBDevIntEn ,USB_BASE_ADDR+0x04);
wo0(usb,USBDevIntClr,USB_BASE_ADDR+0x08);
wo0(usb,DEV_INT_SET ,USB_BASE_ADDR+0x0C); //USBDevIntSet in datasheet
wo0(usb,USBDevIntPri,USB_BASE_ADDR+0x2C);

/* Endpoint Interrupt Registers */
ro0(usb,USBEpIntSt ,USB_BASE_ADDR+0x30);
rw0(usb,USBEpIntEn ,USB_BASE_ADDR+0x34);
wo0(usb,USBEpIntClr,USB_BASE_ADDR+0x38);
wo0(usb,EP_INT_SET ,USB_BASE_ADDR+0x3C); //USBEpIntSet in datasheet
wo0(usb,USBEpIntPri,USB_BASE_ADDR+0x40);

/* Endpoint Realization Registers */
rw0(usb,USBReEp    ,USB_BASE_ADDR+0x44);
wo0(usb,USBEpInd   ,USB_BASE_ADDR+0x48);
rw0(usb,USBMaxPSize,USB_BASE_ADDR+0x4C);

/* Data Transfer Registers */
ro0(usb,USBRxData,USB_BASE_ADDR+0x18);
ro0(usb,USBTxData,USB_BASE_ADDR+0x1C);
wo0(usb,USBRxPLen,USB_BASE_ADDR+0x20);
wo0(usb,USBTxPLen,USB_BASE_ADDR+0x24);
rw0(usb,USBCtrl  ,USB_BASE_ADDR+0x28);

/* Command Reagisters */
wo0(usb,USBCmdCode,USB_BASE_ADDR+0x10);
ro0(usb,USBCmdData,USB_BASE_ADDR+0x14);

/* DMA Registers */
ro0(usb,DMA_REQ_STAT    ,USB_BASE_ADDR+0x50);  //USBDMARSt in datasheet
wo0(usb,DMA_REQ_CLR     ,USB_BASE_ADDR+0x54);  //USBDMARClr
wo0(usb,DMA_REQ_SET     ,USB_BASE_ADDR+0x58);  //USBDMARSet
rw0(usb,UDCA_HEAD       ,USB_BASE_ADDR+0x80);  //USBUDCAH
ro0(usb,EP_DMA_STAT     ,USB_BASE_ADDR+0x84);  //USBEpDMASt
wo0(usb,EP_DMA_EN       ,USB_BASE_ADDR+0x88);  //USBEpDMAEn
wo0(usb,EP_DMA_DIS      ,USB_BASE_ADDR+0x8C);  //USBEpDMADis
ro0(usb,DMA_INT_STAT    ,USB_BASE_ADDR+0x90);  //USBDMAIntSt
rw0(usb,DMA_INT_EN      ,USB_BASE_ADDR+0x94);  //USBDMAIntEn
ro0(usb,EOT_INT_STAT    ,USB_BASE_ADDR+0xA0);  //USBEoTIntSt
wo0(usb,EOT_INT_CLR     ,USB_BASE_ADDR+0xA4);  //USBEoTIntClr
wo0(usb,EOT_INT_SET     ,USB_BASE_ADDR+0xA8);  //USBEoTIntSet
ro0(usb,NDD_REQ_INT_STAT,USB_BASE_ADDR+0xAC);  //USBNDDRIntSt
wo0(usb,NDD_REQ_INT_CLR ,USB_BASE_ADDR+0xB0);  //USBNDDRIntClr
rw0(usb,NDD_REQ_INT_SET ,USB_BASE_ADDR+0xB4);  //USBNDDRIntSet
ro0(usb,SYS_ERR_INT_STAT,USB_BASE_ADDR+0xB8);  //USBSysErrIntSt
wo0(usb,SYS_ERR_INT_CLR ,USB_BASE_ADDR+0xBC);  //USBSysErrIntClr
wo0(usb,SYS_ERR_INT_SET ,USB_BASE_ADDR+0xC0);  //USBSysErrIntSet
rw0(usb,MODULE_ID       ,USB_BASE_ADDR+0xFC);

