import re

fnt_file = open('fip-fonts.fnt', 'r')
all_lines = fnt_file.readlines()

header_file = open('FIPFonts.h', 'w')

header_file.write("// Generated file. Do not edit!\n")
header_file.write("// See details in FIP-SDK/fonts/readme.txt\n") 
header_file.write("#include <map>\n") 

header_types = "struct Font {\n\
    Font()\n\
    {\n\
        x = 0; y = 0; width = 0; height = 0;\n\
    }\n\
    Font(int _x, int _y, int _width, int _height)\n\
    {\n\
        x = _x; y = _y; width = _width; height = _height;\n\
    }\n\
    int x;\n\
    int y;\n\
    int width;\n\
    int height;\n\
};\n\
\n\
std::map<int,Font> fip_font_positions;\n\
void init_fip_fonts(){\n"

header_file.write(header_types)
FONT_HEIGHT = 0
MAX_FONT_WIDTH = 0
BMP_WIDTH = 0
BMP_HEIGHT = 0
CHAR_ASCII_ID_MIN = 255
CHAR_ASCII_ID_MAX = 0

for line in all_lines:
    #common lineHeight=16 base=13 scaleW=256 scaleH=256 pages=1 packed=0 alphaChnl=0 redChnl=3 greenChnl=3 blueChnl=3
    result = re.search(r"common lineHeight=([0-9]+).*scaleW=([0-9]+)\s+scaleH=([0-9]+)\s+.*", line)
    if result != None:
        FONT_HEIGHT = int(result.group(1))
        BMP_WIDTH = int(result.group(2))
        BMP_HEIGHT = int(result.group(3))

    #char id=32   x=252   y=17    width=3     height=16    xoffset=-1    yoffset=0     xadvance=4     page=0  chnl=15
    result = re.search(r"char id=([0-9]+)\s+x=([0-9]+)\s+y=([0-9]+)\s+width=([0-9]+)\s+height=([0-9]+).*", line)
    if result != None:
        header_file.write("    fip_font_positions["+str(result.group(1))+"] = Font("+str(result.group(2))+","+str(result.group(3))+","+str(result.group(4))+","+str(result.group(5))+");\n")
        if int(result.group(4)) > MAX_FONT_WIDTH:
            MAX_FONT_WIDTH = int(result.group(4))
        if int(result.group(1)) < CHAR_ASCII_ID_MIN:
            CHAR_ASCII_ID_MIN = int(result.group(1))
        if int(result.group(1)) > CHAR_ASCII_ID_MAX:
            CHAR_ASCII_ID_MAX = int(result.group(1))


header_file.write("}\n")

header_file.write("const int FIP_FONT_FILE_WIDTH="+str(BMP_WIDTH)+";\n")
header_file.write("const int FIP_FONT_FILE_HEIGHT="+str(BMP_HEIGHT)+";\n")
header_file.write("const int FIP_FONT_HEIGHT="+str(FONT_HEIGHT)+";\n")
header_file.write("const int FIP_MAX_FONT_WIDTH="+str(MAX_FONT_WIDTH)+";\n")
header_file.write("const int FIP_MAX_FONT_ASCII_ID="+str(CHAR_ASCII_ID_MAX)+";\n")
header_file.write("const int FIP_MIN_FONT_ASCII_ID="+str(CHAR_ASCII_ID_MIN)+";\n")
