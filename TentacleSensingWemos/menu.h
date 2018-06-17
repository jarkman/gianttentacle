#ifndef MENU_H
#define MENU_H

/*
 * One Menu represents one entry in a menu tree.
 * 
 * One Menu might be:
 * 
 *  - A top-level page 
 *      navigate between sibling top-level pages with encoder up/down while no sub-menu is selected
 *      
 *  - A child page 
 *      navigate to a child page by encoder-down on parent, then click 
 *      navigate between sibling child pages with encoder up/down while no sub-menu is selected
 *      navigate back to parent with encoder click while no sub-menu is selected
 *      
 *  - A variable value
 *      start adjusting value with click
 *      adjust value with encoder up/down
 *      stop adjusting value with click
 *      you can't navigate to a variable val, they are painted by their parent page
 *  
 */
class Menu
{
  public:
  String name;
  Menu*children; // array of children or null
  Menu*parent;  // parent so we can go back up, or null

  void*client; // object this refers to 
  int id;      // unique id for callbackery
  
  int currentChosen; // index of chosen child, which we'll navigate to if clicked, 
                      // or -1 if we are at the start and have not chosen anything, in which case a click takes us to our parent
  
  Menu(String _name, void*_client, int _id);
  void addChild( Menu *child);

  void buttonClick();
  void increment(int delta);
  
};

class MenuHolder
{
  public:
  Menu*root; // root of tree of all the menus
  
  Menu*current; // currently active Menu object

  void buttonClick();
  void increment(int delta);
  
};
#endif MENU_H

