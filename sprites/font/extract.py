#!/usr/bin/env python3

import sys
import os
import json
from PIL import Image

argc = len(sys.argv)

if argc<3:
    print('usage: ./extract <font-basename> <output-dir>')
    sys.exit()

#---- get arguments
basename = sys.argv[1]
outdir = sys.argv[2]

#---- make output folder if DNE
if not os.path.exists(outdir):
    os.mkdir(outdir)

#---- read input

# list of chars
chars = []

# dict: char_a -> (advance, { char_b -> kernings })
advance_kernings = {}

with open(basename+'.fnt', 'r') as f:
    lines = f.readlines()
    for line in lines:
        tokens = line.strip().split()

        if (tokens[0]=='char'):
            ascii_id = tokens[1][3:]
            chars.append({
                'ascii_id': ascii_id,
                'x': int(tokens[2][2:]),
                'y': int(tokens[3][2:]),
                'width': int(tokens[4][6:]),
                'height': int(tokens[5][7:]),
                'xoffset': int(tokens[6][8:]),
                'yoffset': int(tokens[7][8:])
            })
            # insert xadvance
            advance_kernings[ascii_id] = (int(tokens[8][9:]), {})

        if (tokens[0]=='kerning'):
            # read from input
            first = tokens[1][6:]
            second = tokens[2][7:]
            kerning = tokens[3][7:]
            # get current kernings list for this char
            ( advance, kernings ) = advance_kernings[first]
            kernings[second] = kerning
            # store back after modification
            advance_kernings[first] = ( advance, kernings )

# dump advance_kearnings
with open( basename + '.kern', 'w+' ) as outfile:
    json.dump(advance_kernings, outfile)

#---- read original texture
tex = Image.open(basename+".png")

#---- save new texture for each char
for info in chars:
    (anchorX, anchorY) = (-info['xoffset'], -info['yoffset'])
    bbox = (info['x'], info['y'], info['x']+info['width'], info['y']+info['height'])
    out_tex = tex.crop(bbox)
    out_tex.save(outdir + "/" + info['ascii_id'] + "_" + str(anchorX) + "_" + str(anchorY) + ".png")


print('done.')
