
OS := $(shell uname)

ifeq ($(OS), Linux)
	GIMP=gimp
else
	GIMP=/Applications/GIMP.app/Contents/MacOS/gimp
endif

../dist/solidarity.png ../dist/solidarity.atlas : sprites/	
	./pack-sprites ../dist/solidarity sprites/*
