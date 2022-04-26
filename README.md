# CustomQuest
Modular turn-based-RPG-like battle simulator with JSON configuration files.

## Description  
**CustomQuest** is a simple game that lets you battle aganist monsters with a simple RPG-like turn-based mechanic. 

But it's not just that!  
**CustomQuest** is _modular_, meaning _you_ can create your **characters**, your **enemies**, your **items** and your **movesets** via JSON files.  

Personalize your game with your custom profiles and try out different strategy, all in your terminal! 

## Demo visuals

_Coming soon_

## Installation
To install **CustomQuest**, run these commands:

### Manual
```
git clone https://github.com/Haruno19/CustomQuest
cd CustomQuest
make
sudo make install
```
Install directoy: ``/usr/local/share/customquest/``.

## Controls
To play **CustomQuest**, just run the command ``customquest``, as simple as that!

Use the ``Arrow Keys`` and ``Enter`` to move through the game menus, and, while fighting, go back with ``b``.

## Available profiles
### Heroes  
```
Hero
```

### Enemies  
```
Slime
Orange Slime
```

### Inventories  
```
Basic Inventory
```

## Game Profiles
All the game profiles are stored as JSON files in the ``res/`` folder, each type in its subfolder, ``heores/``, ``enemies/``, ``items/``. 
  
You can also add ASCII Arts for your monsters! 
Just create a file containing the ASCII Art with the same name as the monster, and place it in the ``res/arts`` folder. 

### JSON Format
_Coming soon_
### How to add your game profiles
To add your custom JSON files, you can place them in the ``res/`` folder before running ``sudo make install``, or place them in ``/usr/local/share/customquest/`` if you've already installed **CustomQuest**.

_I'll add more detailed instructions on how to create and insteall your own game profiels ASAP_
  
## Credits & Dependencies

### JSON for Modern C++
To work easly with JSON files, **CustomQuest** relies on [JSON for Modern C++](https://github.com/nlohmann/json) by [Niels Lohmann](https://github.com/nlohmann).   
Although relying on external libraries, no dependencies are needed to install **CustomQuest**, since said library is already included in this repository (``src/include/json.hpp``) via the [single_include](https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp) version provided by the author, in order to make it easier for you, the end user, to enjoy **CustomQuest**.
