#include "string.h"
#include "stdlib.h"
#include "frameworks/list.h"
#include "frameworks/ledScreen.h"
#include "frameworks/crc.h"

uint8_t ledScreen_hw_send(uint8_t byte);

static uint16_t const g_crcTable[256] = {
	0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241, 0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1,
	0XC481, 0X0440, 0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40, 0X0A00, 0XCAC1, 0XCB81, 0X0B40,
	0XC901, 0X09C0, 0X0880, 0XC841, 0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40, 0X1E00, 0XDEC1,
	0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41, 0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
	0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040, 0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1,
	0XF281, 0X3240, 0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441, 0X3C00, 0XFCC1, 0XFD81, 0X3D40,
	0XFF01, 0X3FC0, 0X3E80, 0XFE41, 0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840, 0X2800, 0XE8C1,
	0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41, 0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
	0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640, 0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0,
	0X2080, 0XE041, 0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240, 0X6600, 0XA6C1, 0XA781, 0X6740,
	0XA501, 0X65C0, 0X6480, 0XA441, 0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41, 0XAA01, 0X6AC0,
	0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840, 0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
	0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40, 0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1,
	0XB681, 0X7640, 0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041, 0X5000, 0X90C1, 0X9181, 0X5140,
	0X9301, 0X53C0, 0X5280, 0X9241, 0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440, 0X9C01, 0X5CC0,
	0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40, 0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
	0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40, 0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0,
	0X4C80, 0X8C41, 0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641, 0X8201, 0X42C0, 0X4380, 0X8341,
	0X4100, 0X81C1, 0X8081, 0X4040
};

void ledScreen_initPackage(ledScreen_package_t* package) {
	//TODO
	struct ledScreen_package_t* package1;
	package1 = package;
	package1->head.descAddress = 0x0001;
	package1->head.sourceAddress = 0x8000;
	package1->head.reserved0 = 0x00000000;
	package1->head.checkMode = 0x00;
	package1->head.displayMode = 0x01;
	package1->head.deviceType = 0x51;
	package1->head.protocolVersion = 0x02;
	package1->head.dataLength = 0x007D;

	package1->info.commandGroup = 0xa3;
	package1->info.command = 0x06;
	package1->info.response = 0x01;
	package1->info.processMode = 0x00;
	package1->info.reserved0 = 0x00;
	package1->info.deleteAreaNumber = 0x00;
	package1->info.areaNumber = 0x01;
	package1->info.areaDataLength = 0x0074;
	package1->info.areaType = 0x06;
	package1->info.areaX = 0x0000;
	package1->info.areaY = 0x0000;
	package1->info.areaWidth = 0x0060;
	package1->info.areaHeight = 0x0020;
	package1->info.dynamicAreaLoc = 0x00;
	package1->info.lineSize = 0x00;
	package1->info.runmode = 0x01;
	package1->info.timeout = 0x0003;
	package1->info.soundMode = 0x00;
	package1->info.reserved1 = 0x0000;
	package1->info.singleLine = 0x02;
	package1->info.newLine = 0x02;
	package1->info.displayModeEx = 0x05;
	package1->info.exitMode = 0x05;
	package1->info.speed = 0x0a;
	package1->info.stayTime = 0x0a;
	package1->info.dataLength = 0x00000059;
};

void ledScreen_setData(ledScreen_package_t* package, uint8_t* data, uint32_t size) {
	package->data = data;
	package->info.dataLength = size;
	package->head.dataLength = size + sizeof(package->info);
}

uint32_t ledScreen_seralizeSize(ledScreen_package_t * package) {
	struct ledScreen_package_t* package1;
	package1 = package;
	return sizeof(package1->head)+sizeof(package1->info) + package1->info.dataLength + 2;
}

//static uint16_t crc(uint8_t* buffer,int offset) {//20180321
static uint16_t doCrc(uint8_t* buffer, uint32_t size) {
 uint16_t tempcrc = 0x0;
 uint16_t temp = 0;
 fors(size) {
	temp = ((tempcrc & 0xff) ^ (buffer[i] & 0xff));
	tempcrc = ((tempcrc>>8) & 0xff) ^ g_crcTable[temp];
 }
 return (tempcrc>>8)|(tempcrc<<8);
}
uint8_t out1[170];
	uint32_t offset1 = 0;
uint32_t ledScreen_seralize(ledScreen_package_t* package, uint8_t* out) {
	uint32_t offset = 0;
	memcpy(out+offset, &package->head, sizeof(package->head));
	offset += sizeof(package->head);
		 	 offset1 = offset;
	memcpy(out+offset, &package->info, sizeof(package->info));
	offset += sizeof(package->info);
		 	 offset1 = offset;
	memcpy(out+offset, package->data, package->info.dataLength);
	offset += package->info.dataLength;
		 	 offset1 = offset;
	 fors(offset) {
		out1[i]=*(out+i);
 }

	uint16_t crc = doCrc(out,offset);
	uint16_t crc2 = crc_modbus(out,offset);
	out[offset++] = crc >> 8;
	out[offset++] = crc & 0xff;

 	 offset1 = offset;
	return offset;
}

static void send(uint8_t* buffer, uint32_t size) {
	fors(8) {
		ledScreen_hw_send(0xa5);
	}
	fortas(uint8_t, buffer, size) {
		if (v == 0xa5) {
			ledScreen_hw_send(0xa6);
			ledScreen_hw_send(0x02);
		} else if (v == 0xa6) {
			ledScreen_hw_send(0xa6);
			ledScreen_hw_send(0x01);
		} else if (v == 0x5a) {
			ledScreen_hw_send(0x5b);
			ledScreen_hw_send(0x02);
		} else if (v == 0x5b) {
			ledScreen_hw_send(0x5b);
			ledScreen_hw_send(0x01);
		} else {
			ledScreen_hw_send(v);
		}
	}
	fors(1) {
		ledScreen_hw_send(0x5a);
	}
}

uint8_t ledScreen_display(ledScreen_package_t* package) {
	uint8_t* buffer = (uint8_t*)malloc(ledScreen_seralizeSize(package));
	if (buffer) {
		send(buffer, ledScreen_seralize(package, buffer));
		free(buffer);
		return 1;
	}
	return 0;
}

// 00 //ProcessMode
// 00 //Reserved
// 00 //DeleteAreaNum  DeleteAreaId不发送
// 01 //AreaNum
// 21 00 //AreaDataLen0
// 06 //AreaType   动态区类型在6以上
// 00 00 //AreaX   0
// 00 00 //AreaY   0
// 08 00 //AreaWidth   64
// 20 00 //AreaHeight   32
// 00 //DynamicAreaLoc   0
// 00 //Lines_sizes
// 00 //Runmode
// 02 00 //Timeout
// 00 //SoundMode   SoundPerson\SoundVolume\SoundSpeed\SoundDataLen\SoundData不发送
// 00 00 //Reserved
// 01 //SingleLine  单行
// 01 //NewLine   手动换行
// 01 //DisplayMode  静止
// 00 //ExitMode
// 00 //Speed
// 0a //StayTime
// 06 00 00 00 //DataLen
// 48 65 6c 6c 6f 21 //Data   Hello!
//void ledScreen_display(uint8_t* arg,uint8_t size)
//{
//	uint8_t* arr = arg;
//	uint8_t arr1[size];
//	for(int i=0;i<size;i++)
//	{
//     arr1[i] = arr[i];
//	}
//	for(int j=0;j<=20;j=j+4)
//	{
//		if(arr1[j] == 0x30)
//		{
//			arr1[j] = 0x00;
//			if(arr1[j+1] == 0x30)
//			{
//					arr1[j+1] = 0x00;
//					if(arr1[j+2] == 0x30)
//					{
//						arr1[j+2] = 0x00;
//					}
//			}
//		}
// }


//	  /*txDataBuf�е�����--��ҪУ����ֵ*/
//		uint8_t txDataBuf[LED_BUFFER_SIZE_1] = {
//			0x01, // area count
//			0x00,0x00, //
//			0x80,
//			0x00,0x00,
//			0x00,0x00,0x00,0x01,0x51,0x02,0x44,0x00,0xa3,0x06,0x01,0x00,0x00,
//		0x00,0x01,0x74,0x00,0x06,0x00,0x00,0x00,0x00,0x60,0x00,0x20,0x00,0x00,0x00,0x00,0x02,0x00,
//		0x00,0x00,0x00,0x02,0x02,0x06,0x00,0x0a,0x0a,0x59,0x00,0x00,0x00,0x50,0x4D,0x32,0x2E,0x35,
//		0x3a,arr1[0],arr1[1],arr1[2],arr1[3],0x20,0x75,0x67,0x2F,0x6D,0x33,0x50,0x4D,0x31,0x30,0x20,
//		0x3a,arr1[4],arr1[5],arr1[6],arr1[7],0x20,0x75,0x67,0x2F,0x6D,0x33,0xCE,0xC2,0xB6,0xC8,0x3A,
//		arr1[8],arr1[9],arr1[10],arr1[11],0x00,0xA1,0xE6,0x20,0x20,0xCA,0xAA,0xB6,0xC8,0x3A,arr1[12],
//		arr1[13],arr1[14],arr1[15],0x20,0x25,0x52,0x48,0xB7,0xE7,0xCF,0xF2,0x3A,arr1[16],arr1[17],
//		arr1[18],arr1[19],0xA1,0xE3,0x20,0x20,0xB7,0xE7,0xCB,0xD9,0x3A,arr1[20],arr1[21],arr1[22],
//		arr1[23],0x20,0x6D,0x2F,0x73,0x00,0x00};//5


//		uint8_t crc_l = my_CRC(txDataBuf, sizeof(txDataBuf)/sizeof(uint8_t))&0xff;
//		uint8_t crc_h = my_CRC(txDataBuf, sizeof(txDataBuf)/sizeof(uint8_t))>>8;


//	  /*txDataBuf_crc�е����ݰ���У��ֵ*/
//		uint8_t txDataBuf_crc[LED_BUFFER_SIZE_2] = {0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0x01,
//		0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x01,0x51,0x02,0x44,0x00,0xa3,0x06,0x01,0x00,0x00,
//		0x00,0x01,0x74,0x00,0x06,0x00,0x00,0x00,0x00,0x60,0x00,0x20,0x00,0x00,0x00,0x00,0x02,0x00,
//		0x00,0x00,0x00,0x02,0x02,0x06,0x00,0x0a,0x0a,0x59,0x00,0x00,0x00,0x50,0x4D,0x32,0x2E,0x35,
//		0x3a,arr1[0],arr1[1],arr1[2],arr1[3],0x20,0x75,0x67,0x2F,0x6D,0x33,0x50,0x4D,0x31,0x30,0x20,
//		0x3a,arr1[4],arr1[5],arr1[6],arr1[7],0x20,0x75,0x67,0x2F,0x6D,0x33,0xCE,0xC2,0xB6,0xC8,0x3A,
//		arr1[8],arr1[9],arr1[10],arr1[11],0x00,0xA1,0xE6,0x20,0x20,0xCA,0xAA,0xB6,0xC8,0x3A,arr1[12],
//		arr1[13],arr1[14],arr1[15],0x20,0x25,0x52,0x48,0xB7,0xE7,0xCF,0xF2,0x3A,arr1[16],arr1[17],
//		arr1[18],arr1[19],0xA1,0xE3,0x20,0x20,0xB7,0xE7,0xCB,0xD9,0x3A,arr1[20],arr1[21],arr1[22],
//		arr1[23],0x20,0x6D,0x2F,0x73,0x00,0x00,crc_h,crc_l,0x5a};//8



//    /*buffer�е������ǽ�txDataBuf_crc�е�ֵ��ת��֮����ֵ�����͸�led���ƿ�������ʾ��*/
//		uint8_t buffer[LED_BUFFER_SIZE_3] = {0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0x01,
//		0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x01,0x51,0x02,0x44,0x00,0xa3,0x06,0x01,0x00,0x00,
//		0x00,0x01,0x74,0x00,0x06,0x00,0x00,0x00,0x00,0x60,0x00,0x20,0x00,0x00,0x00,0x00,0x02,0x00,
//		0x00,0x00,0x00,0x02,0x02,0x06,0x00,0x0a,0x0a,0x59,0x00,0x00,0x00,0x50,0x4D,0x32,0x2E,0x35,
//		0x3a,arr1[0],arr1[1],arr1[2],arr1[3],0x20,0x75,0x67,0x2F,0x6D,0x33,0x50,0x4D,0x31,0x30,0x20,
//		0x3a,arr1[4],arr1[5],arr1[6],arr1[7],0x20,0x75,0x67,0x2F,0x6D,0x33,0xCE,0xC2,0xB6,0xC8,0x3A,
//		arr1[8],arr1[9],arr1[10],arr1[11],0x00,0xA1,0xE6,0x20,0x20,0xCA,0xAA,0xB6,0xC8,0x3A,arr1[12],
//		arr1[13],arr1[14],arr1[15],0x20,0x25,0x52,0x48,0xB7,0xE7,0xCF,0xF2,0x3A,arr1[16],arr1[17],
//		arr1[18],arr1[19],0xA1,0xE3,0x20,0x20,0xB7,0xE7,0xCB,0xD9,0x3A,arr1[20],arr1[21],arr1[22],
//		arr1[23],0x20,0x6D,0x2F,0x73,0x00,0x00,crc_h,crc_l,0x5a};

//    ledScreen_ESC(txDataBuf_crc,buffer);

//  	HAL_UART_Transmit(&huart1, (uint8_t*)buffer, LED_BUFFER_SIZE_3, 50);

//}
