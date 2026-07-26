# OpenGL Project

## building
to build the project when you are **inside the build** folder

### with Make
```shell
cmake ..
make
```

### with Ninja
```shell
cmake .. -G Ninja
ninja
```

## running
also from inside the build folder

```shell
./opengl_project 
```

## keybinds

### movement
`WASD`: standard movement
- `W`: forward
- `A`: left
- `S`: reverse
- `D`: right

`SPACE`: up

`CTRL`: down

### camera
`1`, `2` & `3`: switch between respectively no effect, Gaussian Blur & Edge Detection

`C`: switch between freeroam camera and the POV of the bee

- default = freeroam

`SCROLL_UP` / `SCROLL_DOWN`: zoom in/out

### functions
`B`: switch bloom on/off

- default = on

`Q`: terminate program

`R`: switch lightpoints on/off

- default = on

`T`: switch to the other track

`O`: switch overlay on/off

- default = off

- `G`: switch chroma key (green screen) on/off
  - default = on
