# ssd1306_32bit
SSD1306 fast library with support [X-GLCD](https://documentation.help/MikroElektronika-GLCD-Font-Creator/x_glcd_library.htm) fonts for 32-bit MCUs.

Быстрая библиотека дисплея SSD1306, использующая шрифты X-GLCD. Высокая скорость достигается за счет использования вертикальной адресации и 32-битных операций с буфером дисплея. 

## Содержание
- [Подключение](#install)
- [Инициализация](#init)
- [Использование](#usage)
- [Добавление шрифтов](#fonts)
- [Графика](#graphics)

<a id="install"></a>
## Подключение
1. Скопировать файлы `.h` и `.c` в каталог с проектом.
2. Отредактировать `#define SSD1306_HEIGHT` в файле `ssd1306_32bit.h`, если высота дисплея 64 пикселя.
3. Добавить `#include "ssd1306_32bit.h"` в файл `main.c`.
4. Выполнить инициализацию.

<a id="init"></a>
## Инициализация
1. Создать и заполнить структуру конфигурации вывода символов.
```c
PrintConfig_t main_print_conf;
main_print_conf.Color = 1;        //0=пиксель черный, 1=пиксель белый
main_print_conf.CursorType = 2;   //0=отсутствует, 1=заливка, 2=черта снизу
main_print_conf.ExtraSpace = 0;   // отступ между символами для пропорциональных шрифтов
main_print_conf.Transparent = 1;  //0=фон контрастный, 1=фон прозрачный
```
2. Создать функцию вывода данных в дисплей.
```c
int i2c_write(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size){
	return HAL_I2C_Mem_Write(&hi2c1,DevAddress,MemAddress,1,pData,Size,100);
}
//int spi_write(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size){
//	HAL_SPI_Transmit (&hspi2, (uint8_t*) &MemAddress, 1, 100);
//	return HAL_SPI_Transmit (&hspi2, pData, Size, 100);
//}
```
3. Создать и заполнить структуру инициализации.
```c
InitConfig_t ssd1306_config;
ssd1306_config.PrintConfig = &main_print_conf;
ssd1306_config.i2c_Address = 0;
ssd1306_config.ssd1603_MemWrite = i2c_write;
//ssd1306_config.ssd1603_MemWrite = spi_write;
```
4. Вызвать функцию инициализации. I2C или SPI уже должны быть проинициализированы и готовы к работе.
```c
ssd1306_Init(&ssd1306_config);
```
<a id="usage"></a>
## Использование
```c
char stmp[10];
int value = 1234;
sprintf(stmp,"%7d",value);
ssd1306_SetPos(0,0);
ssd1306_DrawString(stmp, &Consolas9x16, cursor_pos);
char stmp2="test str";
ssd1306_SetPos(0,16);
ssd1306_DrawString(stmp2, &Consolas9x16, cursor_pos);
ssd1306_UpdateScreen();
```

<a id="fonts"></a>
## Добавление шрифтов
1. Скачать и установить [GLCD Font Creator](https://www.mikroe.com/glcd-font-creator); примеры по работе с программой: [раз](https://kkmspb.ru/development/microcontrollers/fonts/Microelectronica-GLCD-Font-Creator.php), [два](http://we.easyelectronics.ru/lcd_gfx/shrifty-s-glcd-font-creator-na-kolenke.html). 
2. Сгенерировать массив основных символов (обычная латиница - From 32 To 127). Добавить массив в файл `fonts_GLCD.c`.
```c
const unsigned char Consolas9x16_data[] = {
/// data 
};
```
3. Сгенерировать массив дополнительных символов (кириллица - From 1040 To 1103). Добавить массив в файл `fonts_GLCD.c`.
```c
const unsigned char Consolas9x16RU_data[] = {
/// data
};
```
4. Создать и заполнить структуру шрифта в файле `fonts_GLCD.c`,
```c
FontGLCD_t Consolas9x16 = {9,16,32,1, Consolas9x16_data, Consolas9x16RU_data};
```
где: `9` - ширина шрифта, `16` - высота шрифта, `32` - смещение первого символа от начала таблицы (обычно 32), `1` - флаг моноширинного шрифта, `Consolas9x16_data` - массив основных символов, `Consolas9x16RU_data` - массив дополнительных символов.

5. Объявить структуру в файле `fonts_GLCD.h`.
```c
extern FontGLCD_t Consolas9x16;
```
6. Использовать указатель на  шрифт в функциях вывода.
```c
ssd1306_DrawCharFast('A',&Consolas9x16);
```
<a id="graphics"></a>
## Графика
Работа с графикой не предусмотрена в библиотеке. 
- Функционал отрисовки примитивов должен быть реализован отдельно, и вызывать фунцкцию `ssd1306_DrawPixel(int x, int y);`. 
- Отрисовка пользовательских иконок (звонки, смс, уведомления и прочее) делается путем создания шрифта, содаржащего необходимые иконки, и вызова `ssd1306_DrawCharFast()` или `ssd1306_DrawCharUpd()`. Пример - шрифт `RFM_sign24x16`.






