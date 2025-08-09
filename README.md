# Tooltips

Automatically adds tooltips to buttons. Just hover over a button to see one, or hold the button on mobile.
This uses Node IDs to automatically find what the tooltip should be, it is not always correct, mod developers can change what it shows if needed.


Mod devs: if you wish to add a tooltip to your button rather than relying on the node IDs this uses, set the `alphalaneous.tooltips/tooltip` user object on the button to a CCString of what you want to see as the tooltip

Example:

```cpp
btn->setUserObject("alphalaneous.tooltips/tooltip", CCString::create("My Button!"));
```