/*
 * ssd1306_32bit.h
 *
 *  Created on: 29 ��� 2020 �.
 *      Author: Maksim Cherkasov
 *
 *  ������������ ������ GLCD, ������ ��������� fonts_GLCD.h
 */

#include "stdint.h"
#include "fonts_GLCD.h"

#ifndef SSD1306_32BIT_H_
#define SSD1306_32BIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_I2C_ADDR_DEFAULT    	0x78
#define SSD1306_I2C_COMMAND				0x00
#define SSD1306_I2C_DATA				0x40

#define SSD1306_WIDTH           		128
#define SSD1306_HEIGHT					32

#if SSD1306_HEIGHT == 64
typedef uint64_t DispColumn_t;
#else
typedef uint32_t DispColumn_t;
#endif

typedef struct {
	//0=������� ������, 1=������� �����
	unsigned int Color :1; 						// ���� ��������
	//0=��� �����������, 1=��� ����������
	unsigned int Transparent :1;				// ������������
	//0=�����������, 1=�������, 2=����� �����
	unsigned int CursorType :2;					// ��� �������
	unsigned int ExtraSpace :4;					// ������ ����� ��������� ��� ���������������� �������
} PrintConfig_t;

typedef struct {
	uint8_t i2c_Address;						// I2C ����� �������
	PrintConfig_t *PrintConfig;					// ��������� �� ��������� ������������ ������
	int (*ssd1603_MemWrite)(uint16_t DevAddress, uint16_t MemAddress,
			uint8_t *pData, uint16_t Size); 	// ��������� �� ������� ������ I2C
} InitConfig_t;

int ssd1306_Init(InitConfig_t *InitConfig);
int ssd1306_UpdateScreen();
void ssd1306_Clear();
void ssd1306_SetPos(int x, int y);
void ssd1306_DrawPixel(int x, int y);
char ssd1306_DrawCharFast(char ch, FontGLCD_t* Font);
char ssd1306_DrawCharCurs(char ch, FontGLCD_t* Font, uint8_t isCursored);
char ssd1306_DrawString(const char* str, FontGLCD_t* Font, uint32_t CursorPos);
int ssd1306_DrawCharUpd(char ch, FontGLCD_t* Font);
int ssd1306_FlipMirror(uint8_t Mirror_X, uint8_t Mirror_Y);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_32BIT_H_ */
