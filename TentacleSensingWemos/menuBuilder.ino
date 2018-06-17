#include "Menu.h"

MenuHolder menuHolder;

void setupMenu()
{
  Menu*status = new Menu("Status",NULL, STATUS_ID);

  Menu*bellows = new Menu("Bellows", NULL, BELLOWS_ID);
  bellows->addChild( new Menu("Base", &baseBellows, BASE_BELLOWS_ID ));

  bellows->addChild( new Menu("Tip", &tipBellows, TIP_BELLOWS_ID));
 
  Menu*root = new Menu("Root",NULL,ROOT_ID);
  root->addChild(status);
  root->addChild(bellows);

  menuHolder.root = root;
}



