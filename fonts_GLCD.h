/*
 * fonts_GLCD.h
 *
 *  Created on: 26 ��� 2020 �.
 *      Author: Maksim Cherkasov
 *
 * ���������� �� �������� �������: http://we.easyelectronics.ru/lcd_gfx/shrifty-s-glcd-font-creator-na-kolenke.html
 * ���������: https://www.mikroe.com/glcd-font-creator
 * ��� ������� �������� �������� �������� 1040 - 1103
 *
 */

#ifndef FONTS_GLCD_H_
#define FONTS_GLCD_H_

//#include "stm32f0xx_hal.h"

typedef struct {
	const unsigned char FontWidth;		// ������ ������
	const unsigned char FontHeight;		// ������ ������
	const unsigned char TableOffset;	// �������� ������� ������� �� ������ �������; ������ 32
	const unsigned char isMono;			// ���� ������������� ������
	const unsigned char *data_regular;	// ��������� �� ������ �������� ��������
	const unsigned char *data_ru;		// ��������� �� ������ ������� ��������
} FontGLCD_t;

extern FontGLCD_t Courier_New_Bold16x26;
extern FontGLCD_t Consolas9x16;
extern FontGLCD_t RFM_sign24x16;
extern FontGLCD_t RFM_hearts32x32;

#endif /* FONTS_GLCD_H_ */
