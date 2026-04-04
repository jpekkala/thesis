# Connect-4 PopOut
This is the source code for the engine that solved Connect-4 PopOut in my thesis [Computer analysis of Connect-4 PopOut](http://jultika.oulu.fi/Record/nbnfioulu-201405281532)

## Building

The project can be built with or without a GUI.

### Console version

In Ubuntu 24.04:
```
sudo apt install g++
cd nogui
make
```

### GUI version

In Ubuntu 24.04:
```
sudo apt install g++ qtbase5-dev qt5-qmake
qmake
make
```

The Qt project file can be regenerated with:
```
qmake -project "QT+=widgets"
```