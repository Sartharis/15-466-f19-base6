
OS := $(shell uname)

ifeq ($(OS), Linux)
	GIMP=gimp
else
	GIMP=/Applications/GIMP.app/Contents/MacOS/gimp
endif

../dist/solidarity.png ../dist/solidarity.atlas : solidarity/	
	./pack-sprites ../dist/solidarity solidarity/*.png
