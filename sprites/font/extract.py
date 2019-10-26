#!/usr/bin/env python3

import sys
import os
import json
from PIL import Image

argc = len(sys.argv)

if argc<3:
    print('usage: ./extract <font-basename> <output-dir>')
    sys.exit()

# get arguments
basename = sys.argv[1]
outdir = sys.argv[2]

# make output folder if DNE
if not os.path.exists(outdir):
    os.mkdir(outdir)

# read json input
with open(basename+'.json', 'r') as f:
    fileinput = json.load(f)
    characters = fileinput['characters']

# read original texture
tex = Image.open(basename+".png")

# save new texture for each char
for c in characters.keys():
    info = characters[c]
    (anchorX, anchorY) = (info['originX'], info['originY'])
    bbox = (info['x'], info['y'], info['x']+info['width'], info['y']+info['height'])
    out_tex = tex.crop(bbox)
    out_tex.save(outdir + "/" + str(ord(c)) + "_" + str(anchorX) + "_" + str(anchorY) + ".png")

print('done.')
