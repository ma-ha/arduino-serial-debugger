//  Serial Port Debugger/Monitor 
//  by ma-ha 
//  Copyright (c) 2023 ma-ha, MIT License
 
#include <Arduino.h>
#include <U8g2lib.h>

#include "PinChangeInterrupt.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define DISPLAY_TXT 'T'
#define DISPLAY_HEX 'H'
#define DISPLAY_BIN 'B'

#define BAUD_PIN 11
#define MODE_PIN 10

String str[ 8 ];

unsigned long baud = 9600;
char mode = DISPLAY_TXT;

// ============================================================================

void setup(void) {
  Serial.begin( baud ); 

  u8g2.begin();
  u8g2.setFont( u8g2_font_5x8_tf );	

  for ( byte i=0; i< 8 ;i++ ) {
    str[i].reserve(25);
    str[i] = "";
  }
  str[4] = "  SERIAL PORT DEBUGGER";
  str[3] = "          BY";
  str[2] = "    MAKER GARAGE DE";
  str[0] = "";

  // button config
  pinMode( BAUD_PIN, INPUT_PULLUP );
  pinMode( MODE_PIN, INPUT_PULLUP );
   
  attachPinChangeInterrupt( 
    digitalPinToPinChangeInterrupt( BAUD_PIN ), 
    changeBaud, 
    RISING 
  );
  attachPinChangeInterrupt( 
    digitalPinToPinChangeInterrupt( MODE_PIN ), 
    changeMode, 
    RISING   
  );
}

// ============================================================================

boolean changed = true;

void loop(void) {
  readStr();
  if ( changed ) {
    writeOLED();
    changed = false;
  }
}

// ============================================================================

void writeOLED() {
  u8g2.clearBuffer();	
  u8g2.setFont( u8g2_font_5x8_tf );	
  for ( byte i=0; i < 8; i++ ) {
    u8g2.drawStr( 0, 64-7*i, str[i].c_str() );	
  }
  u8g2.setFont( u8g2_font_tinytim_tf );	
  u8g2.drawStr( 100, 5, String( baud ).c_str()  );
  switch (mode) {
    case DISPLAY_TXT: u8g2.drawStr( 90, 5, "T" ); break;
    case DISPLAY_BIN: u8g2.drawStr( 90, 5, "B" ); break;
    case DISPLAY_HEX: u8g2.drawStr( 90, 5, "H" ); break;
    default: u8g2.drawStr( 90, 5, "t" ); break;
  }
  u8g2.sendBuffer();		
}

// ============================================================================

byte strLen = 0;
void readStr() {
  while ( Serial.available() ) {
    char inChar = (char) Serial.read();
    
    switch (mode) {
      case DISPLAY_TXT: 
        strLen ++;
        if ( inChar == '\n' ) {
          newLine();
        } else if ( strLen > 24 ) {
          newLine();
          str[0] += inChar;
        } else {
          str[0] += inChar;
        }
        break;
      case DISPLAY_BIN: 
        if ( strLen > 2 ) {
          newLine();
        }
        str[0] += String( inChar, BIN ) + " ";
        strLen ++;
        break;
      case DISPLAY_HEX: 
        strLen += 3;
        str[0] += String( inChar, HEX ) + " ";
        if ( strLen > 24 ) {
          newLine();
        }
        break;
      default: break;
    }
    changed = true;
  }
}

// ============================================================================

void newLine() {
  //Serial.println( str[0].c_str() );
  for ( byte i = 7; i > 0; i-- ) {
    str[i] = str[i-1];
  }
  str[0] = "";
  strLen = 0;
}

// ============================================================================

void changeBaud() {
  //Serial.print( "changeBaud " );
  if ( baud == 9600 ) {
    baud =  19200;
  } else if ( baud ==  19200 ) {
    baud =  31250;
  } else if ( baud ==  31250 ) {
    baud =  57600;
  } else if ( baud ==  57600 ) {
    baud =  115200;
  } else if ( baud == 115200 ) {
    baud = 9600;
  }
  //Serial.println( baud );
  //Serial.end(); 
  //Serial.begin( baud ); 
  changed = true;
}

// ============================================================================

void changeMode() {
  if ( mode == DISPLAY_TXT ) {
    mode = DISPLAY_HEX;
  } else if ( mode == DISPLAY_HEX) {
    mode = DISPLAY_BIN;    
  } else if ( mode == DISPLAY_BIN ) {
    mode = DISPLAY_TXT;
  }
  //Serial.print( "mode " );
  //Serial.println( mode );
  changed = true;
}
