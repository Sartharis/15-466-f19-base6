# BLENDER="C:\Program Files\Blender Foundation\Blender\blender.exe"
BLENDER=/Applications/Blender.app/Contents/MacOS/Blender

all : \
	../dist/solidarity.pnct \

solidarity.pnct : solidarity.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- solidarity.blend solidarity.pnct

../dist/solidarity.pnct : scene-editor.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- scene-editor.blend:solidarity $@
