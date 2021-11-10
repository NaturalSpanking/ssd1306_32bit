/*
 * ssd1306_32bit.c
 *
 *  Created on: 29 мая 2020 г.
 *      Author: Maksim Cherkasov
 */

#include "ssd1306_32bit.h"

static I2C_HandleTypeDef *i2c_handle;
static int current_x;
static int current_y;

static DispColumn_t display_buffer[SSD1306_WIDTH];

static uint16_t i2c_bus_address;

PrintConfig_t *print_config;

const uint8_t init_sequence[] = {	// Initialization Sequence
	0xAE,								// Display OFF (sleep mode)
	0xA8, ((SSD1306_HEIGHT - 1) & 0x3F),// Set multiplex ratio(1 to 64)
	0xD3, 0x00,							// Set display offset. 00 = no offset
	0x40,								// Set start line address
// поворот и отражение
	//0xA1,								// Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	//0xC8,								// Set COM Output Scan Direction
#if SSD1306_HEIGHT == 64
	0xDA, 0b00010010,					// Set COM pins hardware configuration 0b00000010 прямая 0b00010010 черессточная
#else
	0xDA, 0b00000010,					// Set COM pins hardware configuration 0b00000010 прямая 0b00010010 черессточная
#endif
	0x81, 0x01,							// Set contrast control register
	0xA4,								// Output RAM to Display 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	0xA6,								// Set display mode. A6=Normal; A7=Inverse
	0xD5, 0x80,							// Set display clock divide ratio/oscillator frequency and divide ratio
//настройки адресации
	0x20, 0x01,							// Set Memory Addressing Mode 01=Vertical Addressing Mode
	0x21, 0x00, SSD1306_WIDTH-1, 		// Setup column start and end address !может установить указатель рам в начало
	0x22, 0x00, SSD1306_HEIGHT/8-1, 	// 0x03 Setup page start and end address for Horizontal & Vertical Addressing Mode

	0xD9, 0xf1,							// Set pre-charge period
	0xDB, 0x40,							// Set vcomh 0x20,0.77xVcc
//	0x2e, 								/* 2017-11-15: Deactivate scroll */
	0x8D, 0x14,							// Set DC-DC enable
	0xAF								// Display ON in normal mode
};

/**
 * @brief Инициализация дисплея
 * @note I2C должен быть настроен и готов к передаче данных
 * @param Указатель на используемый I2C
 * @param Адрес дисплея на шине
 * @param Указатель на структуру конфигурации печати
 * @retval Статус операции записи массива конфигурации в дисплей
 */
HAL_StatusTypeDef ssd1306_Init(I2C_HandleTypeDef *hi2c, uint16_t i2c_address, PrintConfig_t *PrintConfig) {
	i2c_bus_address = SSD1306_I2C_ADDR_DEFAULT;
	if (i2c_address) {
		i2c_bus_address = i2c_address;
	}
	if (hi2c->State != HAL_I2C_STATE_READY)
		return HAL_ERROR;
	i2c_handle = hi2c;
	print_config = PrintConfig;
	current_x = 0;
	current_y = 0;
HAL_Delay(100);
	return HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_COMMAND, 1, (uint8_t*) init_sequence,
			sizeof(init_sequence), 100);
}

/**
 * @brief Выгрузка буфера на дисплей
 * @note Дисплей обновляется только этой функцией
 * @param
 * @retval Статус операции записи буфера в дисплей
 */
HAL_StatusTypeDef ssd1306_UpdateScreen() {
	return HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_DATA, 1, (uint8_t*) display_buffer,
	sizeof(display_buffer),100);
}

/**
 * @brief Очистка буфера дисплея
 * @note Дисплей не обновляется
 * @param
 * @retval
 */
void ssd1306_Clear() {
	DispColumn_t buf = print_config->Color ? 0LL : ~0LL;
	DispColumn_t* p_buf = display_buffer; 		// Быстрее чем мемсет
	for (int i = 0; i < SSD1306_WIDTH; i++) {	// Чистка буфера
		*p_buf++ = buf;
	}
}

/**
 * @brief Установка текущей позиции для вывода
 * @note Дисплей не обновляется
 * @param Координаты позиции
 * @retval
 */
void ssd1306_SetPos(int x, int y) {
	current_x = x;
	current_y = y;
}

/**
 * @brief Отрисовка пикселя с указанными координатами текущим цветом из конфигурации печати
 * @note Дисплей не обновляется
 * @param Координаты пикселя
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
 * @brief Быстрая отрисовка символа в текущей позиции
 * @note Дисплей не обновляется
 * @param Выводимый символ
 * @param Указатель на используемый шрифт
 * @retval Выводимый символ в случае успеха, 0 если не хватило ширины дисплея
 */
char ssd1306_DrawCharFast(char ch, FontGLCD_t* Font) {	// быстрая функция
	DispColumn_t char_column; 											// столбец символа
	int bytes_per_column = Font->FontHeight / 8 + (Font->FontHeight % 8 ? 1 : 0); // количество байт на 1 столбец символа
	int bytes_per_char = Font->FontWidth * bytes_per_column + 1;	// количество байт на символ
	int char_index;												// индекс символа в массиве шрифта
	const uint8_t* font_table;
	if (ch >= 0xC0) {		//проверка языка
		font_table = Font->data_ru;
		char_index = (ch - 0xC0) * bytes_per_char;	// индекс символа в массиве шрифтов
	} else {
		font_table = Font->data_regular;
		char_index = (ch - Font->TableOffset) * bytes_per_char;	// индекс символа в массиве шрифтов
	}
	int char_width = font_table[char_index];	// количество столбцов символа

	for (int i = 0, j = 1; i < char_width; i++, j += bytes_per_column) { // бежим по столбцам
		if (current_x + i >= SSD1306_WIDTH) {		// проверка на превышение ширины дисплея
			return 0;
		}
		char_column = 0LL;
		for (int k = 0; k < bytes_per_column; k++) { // заполнение столбца байтами шрифта
			char_column |= font_table[char_index + j + k] << (8 * k);
		}
		if (print_config->Color) {					// вывод столбца в буфер
			display_buffer[current_x + i] |= char_column << current_y;
		} else {
			display_buffer[current_x + i] &= ~(char_column << current_y);
		}
	}
	current_x += Font->isMono ? Font->FontWidth : (char_width + print_config->ExtraSpace); // расстояние между символами
	return ch;
}

/**
 * @brief Отрисовка символа в текущей позиции
 * @note Дисплей не обновляется
 * @param Выводимый символ
 * @param Указатель на используемый шрифт
 * @param Наличие курсора на символе
 * @retval Выводимый символ в случае успеха, 0 если не хватило ширины дисплея
 */
char ssd1306_DrawCharCurs(char ch, FontGLCD_t* Font, uint8_t isCursored) {
	int bytes_per_column = Font->FontHeight / 8 + (Font->FontHeight % 8 ? 1 : 0); // количество байт на 1 столбец символа
	int bytes_per_char = Font->FontWidth * bytes_per_column + 1;					// количество байт на символ
	int char_index;						// индекс символа в массиве шрифта

	const uint8_t* font_table;
	if (ch >= 0xC0) {	//проверка языка
		font_table = Font->data_ru;
		char_index = (ch - 0xC0) * bytes_per_char; // индекс символа в массиве шрифтов
	} else {
		font_table = Font->data_regular;
		char_index = (ch - Font->TableOffset) * bytes_per_char; // индекс символа в массиве шрифтов
	}
	int char_width = Font->isMono ? Font->FontWidth : font_table[char_index]; // количество столбцов символа

	DispColumn_t cursor_mask = 0LL;	// маска курсора
	switch (print_config->CursorType) { // заполнение столбца курсора
	case 2:
		cursor_mask = (~(0x0F << (Font->FontHeight - 4))) | (~0LL << (Font->FontHeight));
		break;
	case 1:
		cursor_mask = ~0LL << (Font->FontHeight);
		break;
	}

	DispColumn_t transp_mask = 0LL;		// маска фона
	if (!print_config->Transparent){	// если не прозрачный - подчищаем
		transp_mask = ~0LL;
		transp_mask >>= (SSD1306_HEIGHT-Font->FontHeight);
		transp_mask <<= current_y;
	}

	DispColumn_t char_column; 			// столбец символа
	for (int i = 0, j = 1; i < char_width; i++, j += bytes_per_column) {	// бежим по столбцам
		if (current_x + i >= SSD1306_WIDTH) {				// проверка на превышение ширины дисплея
			return 0;
		}
		char_column = 0LL;
		for (int k = 0; k < bytes_per_column; k++) { 		// заполнение столбца байтами шрифта
			char_column |= font_table[char_index + j + k] << (8 * k);
		}

		if (isCursored) {	// добавление курсора к столбцу
			char_column = ~char_column;
			char_column ^= cursor_mask;
		}

		if (print_config->Color) {		// вывод столбца в буфер
			display_buffer[current_x + i] &=~transp_mask;
			display_buffer[current_x + i] |= char_column << current_y;
		} else {
			display_buffer[current_x + i] |=transp_mask;
			display_buffer[current_x + i] &= ~(char_column << current_y);
		}
	}
	current_x += Font->isMono ? Font->FontWidth : (char_width + print_config->ExtraSpace); // расстояние между символами
	return ch;
}

/**
 * @brief Отрисовка строки с текущей позиции
 * @note Дисплей не обновляется
 * @param Массив символов
 * @param Указатель на используемый шрифт
 * @param Позиция курсора в строке; 0 - курсора нет
 * @retval 0 в случае успеха; символ, начиная с которого не хватило ширины дисплея
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
 * @brief Отрисовка символа в текущей позиции c последующим обновлением участка дисплея
 * @note Дисплей обновляется
 * @param Выводимый символ
 * @param Указатель на используемый шрифт
 * @retval HAL_OK в случае успеха, ERR если ошибка I2C
 */

HAL_StatusTypeDef ssd1306_DrawCharUpd(char ch, FontGLCD_t* Font) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t command[] = { 0x21, 0x00, SSD1306_WIDTH - 1 };
	command[1] = current_x;		// запоминаем начальный столбец
	ssd1306_DrawCharCurs(ch, Font, 0);	// выводим в буфер
	command[2] = current_x;		//запоминаем конечный столбец
	ret += HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_COMMAND, 1, command, 3, 100); // ставим указатель на начальный столбец
	ret += HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_DATA, 1, // отображаем кол-во столбцов
			(uint8_t*) display_buffer + command[1] * sizeof(DispColumn_t),
			(command[2] - command[1]) * sizeof(DispColumn_t), 100);
	command[1] = 0x00;
	command[2] = SSD1306_WIDTH - 1; // обнуляем
	ret += HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_COMMAND, 1, command, 3, 100); // ставим указатель на начало
	return ret;
}

/**
 * @brief Отражение дисплея
 * @note Отражение по двум осям даст переворот
 * @param Отразить X
 * @param Отразить Y
 * @retval HAL_OK в случае успеха, ERR если ошибка I2C
 */

HAL_StatusTypeDef ssd1306_FlipMirror(uint8_t Mirror_X, uint8_t Mirror_Y){
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t command[] = { 0xA0, 0xC0};
	if(Mirror_X) command[0]=0xA1; // 0xA0 - normal X, 0xA1 - mirror X
	if(Mirror_Y) command[1]=0xC8; // 0xC0 - normal Y, 0xC8 - mirror Y
	ret += HAL_I2C_Mem_Write(i2c_handle, i2c_bus_address, SSD1306_I2C_COMMAND, 1, command, 2, 100);
	return ret;
}
