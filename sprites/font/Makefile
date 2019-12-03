FONT_NAME = neucha

.PHONY: all
all:
	rm -rf chars/
	./extract.py $(FONT_NAME) chars
	mv "$(FONT_NAME).kern" ../../dist
	../pack-sprites "../../dist/$(FONT_NAME)-font" chars/*

extract:
	rm -rf chars/
	./extract.py $(FONT_NAME) chars
	mv "$(FONT_NAME).kern" ../../dist

pack:
	../pack-sprites "../../dist/$(FONT_NAME)-font" chars/*
