BLENDER="C:\Program Files\Blender Foundation\Blender\blender.exe"

all : \
	solidarity.pnct \

solidarity.pnct : solidarity.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- solidarity.blend solidarity.pnct
