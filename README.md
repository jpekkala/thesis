# Connect-4 PopOut
This is the source code for the engine that solved Connect-4 PopOut in my thesis [Computer analysis of Connect-4 PopOut](http://jultika.oulu.fi/Record/nbnfioulu-201405281532)

## Building

The project can be built with or without a GUI.

### Console version

In Ubuntu 18.04:
```
sudo apt install g++
cd nogui
make
```

### GUI version

In Ubuntu 18.04:
```
sudo apt install g++ qt5-default
qmake
make
```

The Qt project file can be regenerated with:
```
qmake -project "QT+=widgets"
```