/*
 * fonts_GLCD.h
 *
 *  Created on: 26 мая 2020 г.
 *      Author: Maksim Cherkasov
 *
 * Инструкция по созданию шрифтов: http://we.easyelectronics.ru/lcd_gfx/shrifty-s-glcd-font-creator-na-kolenke.html
 * Программа: https://www.mikroe.com/glcd-font-creator
 * Для русских символов выбирать диапазон 1040 - 1103
 *
 */

#ifndef FONTS_GLCD_H_
#define FONTS_GLCD_H_

typedef struct {
	const unsigned char FontWidth;		// ширина шрифта
	const unsigned char FontHeight;		// высота шрифта
	const unsigned char TableOffset;	// смещение первого символа от начала таблицы; обычно 32
	const unsigned char isMono;			// флаг моноширинного шрифта
	const unsigned char *data_regular;	// указатель на массив основных символов
	const unsigned char *data_ru;		// указатель на массив русских символов
} FontGLCD_t;

// Тут перечислить нужные шрифты

extern FontGLCD_t Courier_New_Bold16x26;
extern FontGLCD_t Consolas9x16;
extern FontGLCD_t RFM_sign24x16;
extern FontGLCD_t RFM_hearts32x32;

#endif /* FONTS_GLCD_H_ */
