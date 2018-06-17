#include "menu.h"

// identifiers for clients
#define ROOT_ID -1
#define STATUS_ID 0
#define BELLOWS_ID 1
#define BASE_BELLOWS_ID 2
#define TIP_BELLOWS_ID 3


Menu::Menu( String _name, void*_client, int _id)
{
  name = _name;
  client = _client;
  id = _id;
  
}

void Menu::addChild( Menu*child )
{
  child->parent = this;  
// TODO - add to array
}

void Menu::buttonClick()
{
  
}

void Menu::increment(int delta)
{
  
}

void MenuHolder::buttonClick()
{
  
}

void MenuHolder::increment(int delta)
{
  
}

