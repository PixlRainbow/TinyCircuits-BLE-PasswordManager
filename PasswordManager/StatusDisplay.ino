//  2.0.0 26 Oct 2017 Initial update release
//
//  Written by Ben Rose, TinyCircuits http://TinyCircuits.com

uint8_t ble_connection_displayed_state = true;

void updateBLEstatusDisplay(const bool force_redraw) {
  if (ble_connection_state == ble_connection_displayed_state && !force_redraw)
    return;
  ble_connection_displayed_state = ble_connection_state;
  int x = 62;
  int y = 6;
  int s = 2;
  uint8_t color = 0x03;
  if (ble_connection_state)
    color = 0xE0;
  display.drawLine(x, y + s + s, x, y - s - s, color);
  display.drawLine(x - s, y + s, x + s, y - s, color);
  display.drawLine(x + s, y + s, x - s, y - s, color);
  display.drawLine(x, y + s + s, x + s, y + s, color);
  display.drawLine(x, y - s - s, x + s, y - s, color);
}

void displayBattery() {
  int result = 0;
#if defined (ARDUINO_ARCH_AVR)
  //http://forum.arduino.cc/index.php?topic=133907.0
  const long InternalReferenceVoltage = 1100L;
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);
  delay(10);
  ADCSRA |= _BV( ADSC );
  while ( ( (ADCSRA & (1 << ADSC)) != 0 ) );
  result = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L;
  //SerialMonitorInterface.println(result);
  //if(result>440){//probably charging
  uint8_t charging = false;
  if (result > 450) {
    charging = true;
  }
  result = constrain(result - 300, 0, 120);
  uint8_t x = 70;
  uint8_t y = 3;
  uint8_t height = 5;
  uint8_t length = 20;
  uint8_t amtActive = (result * length) / 120;
  uint8_t red, green, blue;
  display.drawLine(x - 1, y, x - 1, y + height, 0xFF); //left boarder
  display.drawLine(x - 1, y - 1, x + length, y - 1, 0xFF); //top border
  display.drawLine(x - 1, y + height + 1, x + length, y + height + 1, 0xFF); //bottom border
  display.drawLine(x + length, y - 1, x + length, y + height + 1, 0xFF); //right border
  display.drawLine(x + length + 1, y + 2, x + length + 1, y + height - 2, 0xFF); //right border
  for (uint8_t i = 0; i < length; i++) {
    if (i < amtActive) {
      red = 63 - ((63 / length) * i);
      green = ((63 / length) * i);
      blue = 0;
    } else {
      red = 32;
      green = 32;
      blue = 32;
    }
    display.drawLine(x + i, y, x + i, y + height, red, green, blue);
  }
#elif defined(ARDUINO_ARCH_SAMD)
  //http://atmel.force.com/support/articles/en_US/FAQ/ADC-example
  SYSCTRL->VREF.reg |= SYSCTRL_VREF_BGOUTEN;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SAMPCTRL.bit.SAMPLEN = 0x1;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->INPUTCTRL.bit.MUXPOS = 0x19;         // Internal bandgap input
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC
  // Start conversion
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;
  // Clear the Data Ready flag
  ADC->INTFLAG.bit.RESRDY = 1;
  // Start conversion again, since The first conversion after the reference is changed must not be used.
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;
  // Store the value
  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Waiting for conversion to complete
  uint32_t valueRead = ADC->RESULT.reg;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  SYSCTRL->VREF.reg &= ~SYSCTRL_VREF_BGOUTEN;
  result = (((1100L * 1024L) / valueRead) + 5L) / 10L;
  uint8_t x = 70;
  uint8_t y = 3;
  uint8_t height = 5;
  uint8_t length = 20;
  uint8_t red, green;
  if (result > 325) {
    red = 0;
    green = 63;
  } else {
    red = 63;
    green = 0;
  }
  display.drawLine(x - 1, y, x - 1, y + height, 0xFF); //left border
  display.drawLine(x - 1, y - 1, x + length, y - 1, 0xFF); //top border
  display.drawLine(x - 1, y + height + 1, x + length, y + height + 1, 0xFF); //bottom border
  display.drawLine(x + length, y - 1, x + length, y + height + 1, 0xFF); //right border
  display.drawLine(x + length + 1, y + 2, x + length + 1, y + height - 2, 0xFF); //right border
  for (uint8_t i = 0; i < length; i++) {
    display.drawLine(x + i, y, x + i, y + height, red, green, 0);
  }
#endif
}
