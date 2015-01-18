#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "lcd_controller.h" 
#include "lcd_screen.h" 
#include "lcd_graphics.h" 


void LcdGotoXY (int x, int y) {
	
	LCD_set_address_pointer (LCD_getBaseText () + y * LCD_getCols () + x); 
}

void LcdClrscr (void) {
	int i;
	
	// Erase text screen
	LCD_set_address_pointer( LCD_getBaseText() );
	for (i=0;i<LCD_getTextScreenSize();i++) {
		LCD_data_write_up( 0x00 ); 
	} 
}
	
int MainLcdInit (void) {
	int i;

	// Compute the size of the screen
	// My model is a DG-24128
	int pixelX = 240;
	int pixelY = 64;
	
	int fontSize = 0; // 0=6x8   1=8x8
	
	LCD_screen_init(pixelX,pixelY, fontSize);

	// MODE SET
	LCD_mode( MODE_OR ); 
	
	// DISPLAY MODE
//	LCD_display_mode( DM_TEXT + DM_GRAPHICS );
	LCD_display_mode (DM_TEXT);

	// Erase text screen
	LCD_set_address_pointer( LCD_getBaseText() );
	for (i=0;i<LCD_getTextScreenSize();i++) {
		LCD_data_write_up( 0x00 ); 
	}
	
	// Erase graphic screen
	LCD_set_address_pointer( LCD_getBaseGraphic() );
	LCD_auto_write_start();
	for (i=0;i<LCD_getGraphicScreenSize();i++) {
		LCD_auto_write( 0x00 ); 
	}
	LCD_auto_write_stop(); 

	return 0;		

} // main

