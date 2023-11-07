import xml.etree.ElementTree as et
import os, sys, argparse, io, struct

nummdl = 1
filesize = 0

def convfile(inp: et.Element, out: io.BufferedWriter, nummdl: int, path: str) -> None:
    print(inp.find('library_geometries/geometry/mesh/source').attrib)
    # out.write(("mdl").encode('utf-8'))
    writeheader(inp, out, path)

def writeheader(inp: et.Element, out: io.BufferedWriter, path: str) -> None:
    # no need to output model id bc only one model
    # out.write((nummdl).to_bytes(1, signed=False))
    # out.write((filesize).to_bytes(4, byteorder='big', signed=True))
    writematerials(inp, path)
    writeobjs(inp, out)

# writes C3D_Mtx type thingy to file which represents material
def writematerials(inp: et.Element, path: str) -> None:
    for mat in inp.find('library_materials'):
        with open(os.path.join(path, mat.attrib['name']) + '.slmtl', 'wb') as out:
            for effect in inp.find('library_effects'):
                if effect.attrib['id'] == mat.find('instance_effect').attrib['url'].split('#')[1]:

                    eff = effect.find('profile_COMMON/technique/phong')
                    if eff == None:
                        eff = effect.find('/profile_COMMON/technique/lambert')

                    if eff.find('ambient/color') != None:
                        out.writelines(struct.pack('f', float((c))) for c in eff.find('ambient/color').text.split(' ')) # ambient colour
                    else:
                        out.writelines(struct.pack('f', float((c))) for c in '0 0 0 1'.split(' ')) # default colour
                    if eff.find('diffuse/color') != None:
                        out.writelines(struct.pack('f', float((c))) for c in eff.find('diffuse/color').text.split(' ')) # diffuse colour
                    else:
                        out.writelines(struct.pack('f', float((c))) for c in '0 0 0 1'.split(' ')) # default colour
                    if eff.find('specular/color') != None:
                        out.writelines(struct.pack('f', float((c))) for c in eff.find('specular/color').text.split(' ')) # specular colour
                    else:
                        out.writelines(struct.pack('f', float((c))) for c in '0 0 0 1'.split(' ')) # default colour
                    if eff.find('emission/color') != None:
                        out.writelines(struct.pack('f', float((c))) for c in eff.find('emission/color').text.split(' ')) # emission colour
                    else:
                        out.writelines(struct.pack('f', float((c))) for c in '0 0 0 1'.split(' ')) # default colour
                    

def writeobjs(inp: et.Element, out: io.BufferedWriter) -> None:
    pass
        # out.write(("obj").encode('utf-8'))
        # mesh = inp.findall('library_geometries').find('mesh')















def process(args):
    print(args.romfs)
    for modelfolder in args.models:
        for root, dirs, models in os.walk(os.path.realpath(modelfolder)):
            for modelfile in models:
                if os.path.splitext(modelfile)[1] == '.dae':
                    model = os.path.join(root, modelfile)
                    print(model)
                    with open(model, 'r') as src:
                        nummdl = 1
                        mdl = et.parse(src).getroot()
                        if args.mirrorpath:
                            if not os.path.exists(os.path.splitext(os.path.join(args.romfs, modelfolder, modelfile))[0] + '.slmdl'):
                                os.makedirs(os.path.join(args.romfs, modelfolder))
                            with open(os.path.splitext(os.path.join(args.romfs, modelfolder, modelfile))[0] + '.slmdl', 'wb') as m:
                                # print(m.name)
                                convfile(mdl, m, nummdl, os.path.join(args.romfs, modelfolder))
                        else:
                            with open(os.path.splitext(os.path.basename(model))[0] + '.slmdl', 'wb') as m:
                                convfile(mdl, m, nummdl, args.romfs)


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