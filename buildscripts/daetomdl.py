import xml.etree.ElementTree as et
import os, sys, argparse, io, struct

nummdl = 1
filesize = 0

def convfile(inp: et.Element, out: io.BufferedWriter) -> None:
    print(inp.find('library_geometries/geometry/mesh/source').attrib)
    out.write(("mdl").encode('utf-8'))
    writeheader(inp, out)

def writeheader(inp: et.Element, out: io.BufferedWriter) -> None:
    out.write((nummdl).to_bytes(1, signed=False))
    out.write((filesize).to_bytes(4, byteorder='big', signed=True))
    writematerials(inp, out)
    writeobjs(inp, out)

# writes C3D_Mtx type thingy to file which represents material
def writematerials(inp: et.Element, out: io.BufferedWriter):
    mat = inp.find('library_effects/effect/profile_COMMON/technique/phong')
    # print(mat.find('ambient/color').text)
    # print(' '.join(c for c in mat.find('ambient/color').text.split(' ')))
    out.writelines(struct.pack('f', float((c))) for c in mat.find('ambient/color').text.split(' ')) # ambient colour
    out.writelines(struct.pack('f', float((c))) for c in mat.find('diffuse/color').text.split(' ')) # diffuse colour
    out.writelines(struct.pack('f', float((c))) for c in mat.find('specular/color').text.split(' ')) # specular colour
    out.writelines(struct.pack('f', float((c))) for c in '0 0 0 1'.split(' ')) # emissive colour

def writeobjs(inp: et.Element, out: io.BufferedWriter):
    for i in range(nummdl):
        out.write(("obj").encode('utf-8'))
        out.write(i.to_bytes(1, signed=False))
        mesh = inp.findall('library_geometries')[i].find('mesh')















def process(args):
    print(args.romfs)
    for modelfolder in args.models:
        for root, dirs, models in os.walk(os.path.realpath(modelfolder)):
            for modelfile in models:
                if os.path.splitext(modelfile)[1] == '.dae':
                    model = os.path.join(root, modelfile)
                    print(model)
                    with open(model, 'r') as src:
                        m = 0
                        mdl = et.parse(src).getroot()
                        if args.mirrorpath:
                            if not os.path.exists(os.path.splitext(os.path.join(args.romfs, modelfolder, modelfile))[0] + '.slmdl'):
                                os.makedirs(os.path.join(args.romfs, modelfolder))
                            with open(os.path.splitext(os.path.join(args.romfs, modelfolder, modelfile))[0] + '.slmdl', 'wb') as m:
                                print(m.name)
                                convfile(mdl, m)
                        else:
                            with open(os.path.splitext(os.path.basename(model))[0] + '.slmdl', 'wb') as m:
                                convfile(mdl, m)


    if 'mirrorpath' in args:
        print("mirroring path")
    print(args.romfs)
    pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DAE model parser')
    parser.add_argument('-r', '--romfs', required=True, help='romfs folder path')
    parser.add_argument('-m', '--models', required=True, nargs='+', help='source model path')
    parser.add_argument('-mp', '--mirrorpath', required=False, help='mirror the source path in romfs', action="store_true")
    args = parser.parse_args()
    process(args)