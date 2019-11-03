
OS := $(shell uname)

ifeq ($(OS), Linux)
	GIMP=gimp
else
	GIMP=/Applications/GIMP.app/Contents/MacOS/gimp
endif

../dist/orders.png ../dist/orders.atlas : sprites-orders/	
	./pack-sprites ../dist/orders sprites-orders/*.png
