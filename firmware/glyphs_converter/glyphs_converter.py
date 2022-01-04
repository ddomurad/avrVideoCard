import sys
from PIL import Image

print(sys.argv)
INPUT_IMAGE_FILE = sys.argv[1]
OUTPUT_FILE = sys.argv[2]


DATA_START_ADDR = int(sys.argv[3], 16)
DATA_ADDR_STEP  = 0x0100


IMAGE = Image.open(INPUT_IMAGE_FILE)
IMG_PIXELS = IMAGE.load()
IMAGE_SIZE = IMAGE.size

HORIZONTAL_GLYPHS_CNT = IMAGE_SIZE[0]//8
VERTICAL_GLYPHS_CNT = IMAGE_SIZE[1]//8
TOTAL_GLYPHS_CNT = HORIZONTAL_GLYPHS_CNT*VERTICAL_GLYPHS_CNT
print("Total generated glyphs: 256")
print(f"Total defined glyphs: {TOTAL_GLYPHS_CNT}" )

GLYPH_TABLE = []
TEMP_GLYPH_TABLE = []
for gy in range(VERTICAL_GLYPHS_CNT):
    for gx in range(HORIZONTAL_GLYPHS_CNT):
        TEMP_GLYPH_TABLE = []
        for c in range(8):
            v = ''
            for r in range(8):
                img_value = IMG_PIXELS[gx*8+r, gy*8+c]
                if isinstance(img_value, tuple) or isinstance(img_value, list):
                    v += '1' if sum(img_value[0:2]) != 0 else '0'
                else:
                    v += '1' if img_value > 10 else '0'
            v = hex(int(v, 2))
            TEMP_GLYPH_TABLE.append(v)
        GLYPH_TABLE.append(TEMP_GLYPH_TABLE)


out_file = open(OUTPUT_FILE, 'w')
for i in range(8):
    out_file.write('.org ' + hex(DATA_START_ADDR + DATA_ADDR_STEP*i) + '\n')
    for ci in range(256):
        if ci%10 == 0:
            out_file.write('\n.db ')
        else:
            out_file.write(', ')
        if ci >= TOTAL_GLYPHS_CNT:
            out_file.write('0x00')
        else:
            out_file.write(GLYPH_TABLE[ci][i])
    out_file.write('\n')