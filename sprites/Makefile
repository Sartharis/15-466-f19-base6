
OS := $(shell uname)

ifeq ($(OS), Linux)
	GIMP=gimp
else
	GIMP=/Applications/GIMP.app/Contents/MacOS/gimp
endif

all : solidarity/ plants/
	./pack-sprites ../dist/solidarity solidarity/*.png
	./pack-sprites ../dist/plants plants/*.png

