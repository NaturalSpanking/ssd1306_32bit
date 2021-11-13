/*
 * ssd1306_32bit.h
 *
 *  Created on: 29 мая 2020 г.
 *      Author: Maksim Cherkasov
 *
 *  Используются шрифты GLCD, читать заголовок fonts_GLCD.h
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
	//0=пиксель черный, 1=пиксель белый
	unsigned int Color :1; 						// цвет пикселей
	//0=фон контрастный, 1=фон прозрачный
	unsigned int Transparent :1;				// прозрачность
	//0=отсутствует, 1=заливка, 2=черта снизу
	unsigned int CursorType :2;					// тип курсора
	unsigned int ExtraSpace :4;					// отступ между символами для пропорциональных шрифтов
} PrintConfig_t;

typedef struct {
	uint8_t i2c_Address;						// I2C адрес дисплея
	PrintConfig_t *PrintConfig;					// Указатель на структуру конфигурации печати
	int (*ssd1603_MemWrite)(uint16_t DevAddress, uint16_t MemAddress,
			uint8_t *pData, uint16_t Size); 	// Указатель на функцию вывода I2C
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
