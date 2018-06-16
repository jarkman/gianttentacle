void setupDisplay()
{
  noMux();
  
  oled.begin();     // Initialize the OLED
  oled.clear(PAGE); // Clear the display's internal memory
  oled.clear(ALL);  // Clear the library's display buffer

   oled.setFontType(0);
  int y = 0; //oled.getLCDHeight();
  int fh = oled.getFontHeight();

  oled.setCursor(0,y);
  oled.print("Morning!");
  oled.display();   


}

void loopDisplay()
{
  noMux();
  
  oled.clear(PAGE); // Clear the display's internal memory
  
  oled.setFontType(0);
  
  switch( uiState )
  {
    case 0: 
      printNodes( );
      break;
    case 1:
      printBellows();
      break;
    default:
      break;
  }
  

  oled.display();   


}
