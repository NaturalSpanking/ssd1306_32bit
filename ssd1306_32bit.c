/*
 * ssd1306_32bit.c
 *
 *  Created on: 29 ��� 2020 �.
 *      Author: Maksim Cherkasov
 */

#include "ssd1306_32bit.h"

static int current_x;
static int current_y;

static DispColumn_t display_buffer[SSD1306_WIDTH];

InitConfig_t *init_config;
PrintConfig_t *print_config;

const uint8_t init_sequence[] = {	// Initialization Sequence
	0xAE,								// Display OFF (sleep mode)
	0xA8, ((SSD1306_HEIGHT - 1) & 0x3F),// Set multiplex ratio(1 to 64)
	0xD3, 0x00,							// Set display offset. 00 = no offset
	0x40,								// Set start line address
// ������� � ���������
	//0xA1,								// Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	//0xC8,								// Set COM Output Scan Direction
#if SSD1306_HEIGHT == 64
	0xDA, 0b00010010,					// Set COM pins hardware configuration 0b00000010 ������ 0b00010010 ������������
#else
	0xDA, 0b00000010,					// Set COM pins hardware configuration 0b00000010 ������ 0b00010010 ������������
#endif
	0x81, 0x01,							// Set contrast control register
	0xA4,								// Output RAM to Display 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	0xA6,								// Set display mode. A6=Normal; A7=Inverse
	0xD5, 0x80,							// Set display clock divide ratio/oscillator frequency and divide ratio
//��������� ���������
	0x20, 0x01,							// Set Memory Addressing Mode 01=Vertical Addressing Mode
	0x21, 0x00, SSD1306_WIDTH-1, 		// Setup column start and end address !����� ���������� ��������� ��� � ������
	0x22, 0x00, SSD1306_HEIGHT/8-1, 	// 0x03 Setup page start and end address for Horizontal & Vertical Addressing Mode

	0xD9, 0xf1,							// Set pre-charge period
	0xDB, 0x40,							// Set vcomh 0x20,0.77xVcc
//	0x2e, 								/* 2017-11-15: Deactivate scroll */
	0x8D, 0x14,							// Set DC-DC enable
	0xAF								// Display ON in normal mode
};

/**
 * @brief ������������� �������
 * @note I2C ������ ���� �������� � ����� � �������� ������
 * @param ��������� �� ��������� �������������
 * @retval ������ �������� ������ ������� ������������ � �������
 */
int ssd1306_Init(InitConfig_t *InitConfig) {
	if (InitConfig == 0) {
		return -1;
	}
	init_config = InitConfig;
	if (init_config->i2c_Address == 0)
		init_config->i2c_Address = SSD1306_I2C_ADDR_DEFAULT;

	print_config = init_config->PrintConfig;
	current_x = 0;
	current_y = 0;
	return init_config->ssd1603_MemWrite(init_config->i2c_Address,
			SSD1306_I2C_COMMAND, (uint8_t*) init_sequence,
			sizeof(init_sequence));
}

/**
 * @brief �������� ������ �� �������
 * @note ������� ����������� ������ ���� ��������
 * @param
 * @retval ������ �������� ������ ������ � �������
 */
int ssd1306_UpdateScreen() {
	return init_config->ssd1603_MemWrite(init_config->i2c_Address,
			SSD1306_I2C_DATA, (uint8_t*) display_buffer, sizeof(display_buffer));
//	return HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_DATA, 1, (uint8_t*) display_buffer,
//	sizeof(display_buffer),100);
}

/**
 * @brief ������� ������ �������
 * @note ������� �� �����������
 * @param
 * @retval
 */
void ssd1306_Clear() {
	DispColumn_t buf = print_config->Color ? 0LL : ~0LL;
	DispColumn_t* p_buf = display_buffer; 		// ������� ��� ������
	for (int i = 0; i < SSD1306_WIDTH; i++) {	// ������ ������
		*p_buf++ = buf;
	}
}

/**
 * @brief ��������� ������� ������� ��� ������
 * @note ������� �� �����������
 * @param ���������� �������
 * @retval
 */
void ssd1306_SetPos(int x, int y) {
	current_x = x;
	current_y = y;
}

/**
 * @brief ��������� ������� � ���������� ������������ ������� ������ �� ������������ ������
 * @note ������� �� �����������
 * @param ���������� �������
 * @retval
 */
void ssd1306_DrawPixel(int x, int y) {
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return;
	}
	if (print_config->Color) {
		display_buffer[x] |= 1LL << (y);
	} else {
		display_buffer[x] &= ~(1LL << (y));
	}
}

/**
 * @brief ������� ��������� ������� � ������� �������
 * @note ������� �� �����������
 * @param ��������� ������
 * @param ��������� �� ������������ �����
 * @retval ��������� ������ � ������ ������, 0 ���� �� ������� ������ �������
 */
char ssd1306_DrawCharFast(char ch, FontGLCD_t* Font) {	// ������� �������
	DispColumn_t char_column; 								// ������� �������
	int bytes_per_column = Font->FontHeight / 8
			+ (Font->FontHeight % 8 ? 1 : 0); // ���������� ���� �� 1 ������� �������
	int bytes_per_char = Font->FontWidth * bytes_per_column + 1;// ���������� ���� �� ������
	int char_index;							// ������ ������� � ������� ������
	const uint8_t* font_table;
	if (ch >= 0xC0) {		//�������� �����
		font_table = Font->data_ru;
		char_index = (ch - 0xC0) * bytes_per_char;// ������ ������� � ������� �������
	} else {
		font_table = Font->data_regular;
		char_index = (ch - Font->TableOffset) * bytes_per_char;	// ������ ������� � ������� �������
	}
	int char_width = font_table[char_index];	// ���������� �������� �������

	for (int i = 0, j = 1; i < char_width; i++, j += bytes_per_column) { // ����� �� ��������
		if (current_x + i >= SSD1306_WIDTH) {// �������� �� ���������� ������ �������
			return 0;
		}
		char_column = 0LL;
		for (int k = 0; k < bytes_per_column; k++) { // ���������� ������� ������� ������
			char_column |= font_table[char_index + j + k] << (8 * k);
		}
		if (print_config->Color) {					// ����� ������� � �����
			display_buffer[current_x + i] |= char_column << current_y;
		} else {
			display_buffer[current_x + i] &= ~(char_column << current_y);
		}
	}
	current_x +=
			Font->isMono ?
					Font->FontWidth : (char_width + print_config->ExtraSpace); // ���������� ����� ���������
	return ch;
}

/**
 * @brief ��������� ������� � ������� �������
 * @note ������� �� �����������
 * @param ��������� ������
 * @param ��������� �� ������������ �����
 * @param ������� ������� �� �������
 * @retval ��������� ������ � ������ ������, 0 ���� �� ������� ������ �������
 */
char ssd1306_DrawCharCurs(char ch, FontGLCD_t* Font, uint8_t isCursored) {
	int bytes_per_column = Font->FontHeight / 8
			+ (Font->FontHeight % 8 ? 1 : 0); // ���������� ���� �� 1 ������� �������
	int bytes_per_char = Font->FontWidth * bytes_per_column + 1;// ���������� ���� �� ������
	int char_index;						// ������ ������� � ������� ������

	const uint8_t* font_table;
	if (ch >= 0xC0) {	//�������� �����
		font_table = Font->data_ru;
		char_index = (ch - 0xC0) * bytes_per_char; // ������ ������� � ������� �������
	} else {
		font_table = Font->data_regular;
		char_index = (ch - Font->TableOffset) * bytes_per_char; // ������ ������� � ������� �������
	}
	int char_width = Font->isMono ? Font->FontWidth : font_table[char_index]; // ���������� �������� �������

	DispColumn_t cursor_mask = 0LL;	// ����� �������
	switch (print_config->CursorType) { // ���������� ������� �������
	case 2:
		cursor_mask = (~(0x0F << (Font->FontHeight - 4)))
				| (~0LL << (Font->FontHeight));
		break;
	case 1:
		cursor_mask = ~0LL << (Font->FontHeight);
		break;
	}

	DispColumn_t transp_mask = 0LL;		// ����� ����
	if (!print_config->Transparent) {	// ���� �� ���������� - ���������
		transp_mask = ~0LL;
		transp_mask >>= (SSD1306_HEIGHT - Font->FontHeight);
		transp_mask <<= current_y;
	}

	DispColumn_t char_column; 			// ������� �������
	for (int i = 0, j = 1; i < char_width; i++, j += bytes_per_column) {// ����� �� ��������
		if (current_x + i >= SSD1306_WIDTH) {// �������� �� ���������� ������ �������
			return 0;
		}
		char_column = 0LL;
		for (int k = 0; k < bytes_per_column; k++) { // ���������� ������� ������� ������
			char_column |= font_table[char_index + j + k] << (8 * k);
		}

		if (isCursored) {	// ���������� ������� � �������
			char_column = ~char_column;
			char_column ^= cursor_mask;
		}

		if (print_config->Color) {		// ����� ������� � �����
			display_buffer[current_x + i] &= ~transp_mask;
			display_buffer[current_x + i] |= char_column << current_y;
		} else {
			display_buffer[current_x + i] |= transp_mask;
			display_buffer[current_x + i] &= ~(char_column << current_y);
		}
	}
	current_x +=
			Font->isMono ?
					Font->FontWidth : (char_width + print_config->ExtraSpace); // ���������� ����� ���������
	return ch;
}

/**
 * @brief ��������� ������ � ������� �������
 * @note ������� �� �����������
 * @param ������ ��������
 * @param ��������� �� ������������ �����
 * @param ������� ������� � ������; 0 - ������� ���
 * @retval 0 � ������ ������; ������, ������� � �������� �� ������� ������ �������
 */
char ssd1306_DrawString(const char* str, FontGLCD_t* Font, uint32_t CursorPos) {
	while (*str) {
		if (ssd1306_DrawCharCurs(*str, Font, !(--CursorPos)) != *str) {
			return *str;
		}
		str++;
	}
	return *str;
}

/**
 * @brief ��������� ������� � ������� ������� c ����������� ����������� ������� �������
 * @note ������� �����������
 * @param ��������� ������
 * @param ��������� �� ������������ �����
 * @retval 0 � ������ ������, ERR ���� ������ I2C
 */

int ssd1306_DrawCharUpd(char ch, FontGLCD_t* Font) {
	int ret = 0;
	uint8_t command[] = { 0x21, 0x00, SSD1306_WIDTH - 1 };
	command[1] = current_x;		// ���������� ��������� �������
	ssd1306_DrawCharCurs(ch, Font, 0);	// ������� � �����
	command[2] = current_x;		//���������� �������� �������
	ret += init_config->ssd1603_MemWrite(init_config->i2c_Address,
			SSD1306_I2C_COMMAND, command, 3); // ������ ��������� �� ��������� �������
	ret += init_config->ssd1603_MemWrite(init_config->i2c_Address,
			SSD1306_I2C_DATA,  // ���������� ���-�� ��������
			(uint8_t*) display_buffer + command[1] * sizeof(DispColumn_t),
			(command[2] - command[1]) * sizeof(DispColumn_t));
	command[1] = 0x00;
	command[2] = SSD1306_WIDTH - 1; // ��������
	ret += init_config->ssd1603_MemWrite(init_config->i2c_Address,
			SSD1306_I2C_COMMAND, command, 3); // ������ ��������� �� ������
	return ret;
}

/**
 * @brief ��������� �������
 * @note ��������� �� ���� ���� ���� ���������
 * @param �������� X
 * @param �������� Y
 * @retval 0 � ������ ������, ERR ���� ������ I2C
 */

int ssd1306_SetMirror(uint8_t Mirror_X, uint8_t Mirror_Y) {
	uint8_t command[] = { 0xA0, 0xC0 };
	if (Mirror_X)
		command[0] = 0xA1; // 0xA0 - normal X, 0xA1 - mirror X
	if (Mirror_Y)
		command[1] = 0xC8; // 0xC0 - normal Y, 0xC8 - mirror Y
	return init_config->ssd1603_MemWrite(init_config->i2c_Address,
	SSD1306_I2C_COMMAND, command, 2);;
}

/**
 * @brief ��������� �������������
 * @note ������ ������������� ������� �� 0 �� 255
 * @param �������������
 * @retval 0 � ������ ������, ERR ���� ������ I2C
 */

int ssd1306_SetContrast(uint8_t Contrast) {
	uint8_t command[] = { 0x81, 0xFF };
	command[1] = Contrast;
	return init_config->ssd1603_MemWrite(init_config->i2c_Address,
	SSD1306_I2C_COMMAND, command, 2);
}
