import xml.etree.ElementTree as et
import os, argparse, io, struct, json, re, math

file_edit_times = {} # stores when the model source files were last edited

def convfile(inp: et.Element, path: str) -> None:
    writematerials(inp, path) # create the slmtl files
    writeobjs(inp, path) # create the slmdl files


# writes C3D_Mtx type thingy to file which represents material (used for default fragment lit dae shader) as well as the diffuse textyre
def writematerials(inp: et.Element, path: str) -> list:
    tex = {}
    for image in inp.findall('library_images/image'):
        tex[image.attrib['id']] = image.find('init_from').text
    for mat in inp.findall('library_materials/material'):
        with open(os.path.join(path, mat.attrib['id']) + '.slmtl', 'wb+') as out:
            print(out.name)
            effect = inp.find('library_effects/effect[@id="' + mat.find('instance_effect').attrib['url'].split('#')[1] + '"]')
            inputs = {}
            samplers = {}
            for param in effect.findall('profile_COMMON/newparam'):
                if param.find('surface') is not None:
                    try:
                        inputs[param.attrib['sid']] = tex[param.find('surface/init_from').text]
                    except KeyError:
                        pass
                elif param.find('sampler2D/source') is not None:
                    samplers[param.attrib['sid']] = param.find('sampler2D/source').text
            out.write(("dae_default_material\0").encode("utf-8"))
            eff = effect.find('profile_COMMON/technique/phong')
            if eff == None:
                eff = effect.find('profile_COMMON/technique/lambert')
            if eff.find('ambient/color') != None:
                out.writelines(struct.pack('f', float(c)) for c in eff.find('ambient/color').text.split(' ')) # ambient colour
            else:
                out.writelines(struct.pack('f', float(c)) for c in '0.2 0.2 0.2 1'.split(' ')) # default ambient lighting is a dark grey
            if eff.find('diffuse/color') != None:
                out.writelines(struct.pack('f', float(c)) for c in eff.find('diffuse/color').text.split(' ')) # diffuse colour
            else:
                out.writelines(struct.pack('f', float(c)) for c in '0.4 0.4 0.4 1'.split(' ')) # default diffuse lighting is light grey
            if eff.find('specular/color') != None:
                out.writelines(struct.pack('f', float(c)) for c in eff.find('specular/color').text.split(' ')) # specular colour
            else:
                out.writelines(struct.pack('f', float(c)) for c in '0.8 0.8 0.8 1'.split(' ')) # default specular color is near white
 
            out.writelines(struct.pack('f', float(c)) for c in '0 0 0 1'.split(' ')) # specular1 is unused by any model formats but required for lighting material

            if eff.find('emission/color') != None:
                out.writelines(struct.pack('f', float(c)) for c in eff.find('emission/color').text.split(' ')) # emission colour
            else:
                out.writelines(struct.pack('f', float(c)) for c in '0 0 0 1'.split(' ')) # default emission is zero
            
            if eff.find('transparent/color') != None:
                out.writelines(struct.pack('f', float(c)) for c in eff.find('transparent/color').text.split(' ')) # transparent colour
                out.write((0).to_bytes(4)) # tells the material it is transparent
            else:
                out.writelines(struct.pack('f', float(c)) for c in '0 0 0 1'.split(' ')) # default transparency is opaque
                out.write((1).to_bytes(4)) # tells the material it is opaque
            if eff.find('diffuse/texture') is not None:
                try:
                    out.write(bytes(os.path.splitext(tex[effect.find('profile_COMMON/newparam[@sid="' + samplers[eff.find('diffuse/texture').attrib['texture']] + '"]/surface/init_from').text])[0].split("_Alb")[0], 'utf-8'))
                except KeyError:
                    continue
                out.write((0).to_bytes(1))

def writeheader(root: et.Element, inp: et.Element, out: io.BufferedWriter, geom: et.Element) -> float:
    out.write(("mdl").encode('utf-8')) # start of header
    out.write(root.find('library_materials/material[@id="' + inp.find('instance_geometry/bind_material/technique_common/instance_material').attrib['target'].split('#')[1] + '"]').attrib['id'].encode('utf-8')) # write material name
    out.write((0).to_bytes(1))
    out.write((int(geom.find('mesh/triangles').attrib['count']) * 3).to_bytes(length=4, byteorder='little')) # number of vertices
    n_attr = 0
    len_attr = []
    t_attr_str = []
    for attrib in geom.findall('mesh/source'):
        n_attr += 1
        len_attr.append(attrib.find('technique_common/accessor').attrib['stride'])
        t_attr_str.append(attrib.find('technique_common/accessor/param').attrib['type'])

    t_attr = [0] * n_attr
    sizevert = 0
    if n_attr > 0:
        for i in range(n_attr) :
            sizevert += int(len_attr[i]) * (4 if t_attr_str[i] == 'float' else (2 if t_attr[i] == 'short' else 1))
            t_attr[i] = (3 if t_attr_str[i] == 'float' else (2 if t_attr_str[i] == 'short' else (1 if t_attr_str[i] == 'unsigned byte' else 0)))

    # size of one vertex
    out.write(sizevert.to_bytes(1))
    
    # num attribs
    out.write(n_attr.to_bytes(1))
    # types of attribs
    out.writelines(int(c).to_bytes(1) for c in t_attr)
    # number of components in each attrib
    out.writelines(int(c).to_bytes(1) for c in len_attr)
    # radius of model

    radius = 0.0

    # float_array@[id=source@[id=vertices->input@[semantic=POSITION].source]->technique_common->accessor.source]
    verts = geom.find('mesh/source/float_array[@id="' + 
        geom.find(
            'mesh/source[@id="' + 
                geom.find(
                    'mesh/vertices/input[@semantic="POSITION"]'
                ).attrib['source'].split('#')[1] + 
            '"]/technique_common/accessor'
        ).attrib['source'].split('#')[1] +
        '"]'
    )

    n_pos = int(verts.attrib['count'])

    positions = [float(c) for c in filter(lambda x: len(x) > 0, verts.text.split(' '))]
    
    avg = [0] * 3
    radius = 0

    for i in range(int(n_pos / 3)):
        avg[0] += (positions[i * 3 + 0] / n_pos) * 3
        avg[1] += (positions[i * 3 + 1] / n_pos) * 3
        avg[2] += (positions[i * 3 + 2] / n_pos) * 3
    
    for i in range(int(n_pos / 3)):
        x = positions[i * 3 + 0] - avg[0]
        y = positions[i * 3 + 1] - avg[1]
        z = positions[i * 3 + 2] - avg[2]
        if x ** 2 + y ** 2 + z ** 2 > radius:
            radius = x ** 2 + y ** 2 + z ** 2 # calculates radius^2 and finds the highest one
    
    out.write(struct.pack('f', math.sqrt(radius)))

    return avg

def writeobjs(inp: et.Element, path: str) -> None: 
    geoms = inp.findall('library_geometries/geometry')
    # parse the scene section (since it's the only place where the material is specified)
    for node in inp.findall('library_visual_scenes/visual_scene/node'): # iterate through all nodes in the first scene (i'm going to ignore having multiple scenes because just why) 
        with open(os.path.join(path, node.attrib['id']) + '.slmdl', 'wb+') as m: 
            try:
                geom = inp.find('library_geometries/geometry[@id="' + node.find('instance_geometry').attrib['url'].split('#')[1] + '"]') # get the geometry tag
            except:
                print(node.attrib)
                exit(1)
            avgpos = writeheader(inp, node, m, geom) # creates the header and also calculates the center of the object for us
            m.write(("obj").encode('utf-8')) # start of object stuff

            m.write((0).to_bytes(1)) # placeholder for bones, currently zero of them

            attribs = geom.findall('mesh/triangles/input')
            offsets = [int(o.attrib['offset']) for o in attribs]
            sources = [o.attrib['source'].split('#')[1] for o in attribs]
            sourceelems = [et.Element] * len(attribs)
            stride = [int] * len(attribs)
            nv = int(geom.find('mesh/triangles').attrib['count']) * 3 # 3 vertices per triangle
            sourcevals = []
            indices = [int(i) for i in filter(lambda x: len(x) > 0,geom.find('mesh/triangles/p').text.split(' '))]

            for i in range(len(attribs)):
                if attribs[i].attrib['semantic'] == 'VERTEX': # vertex source is weird, need to redirect
                    sources[i] = geom.find('mesh/vertices[@id="' + sources[i] + '"]/input').attrib['source'].split('#')[1] # assume that there is only one possible thing in a vertex tag
                sourceelems[i] = geom.find('mesh/source[@id="' + sources[i] + '"]')
                stride[i] = int(sourceelems[i].find('technique_common/accessor').attrib['stride'])
                sourcevals.append([float(val) for val in filter(lambda x: len(x) > 0, sourceelems[i].find('float_array').text.split(' '))])

            t_attr_str = []
            for attrib in sourceelems:
                t_attr_str.append(attrib.find('technique_common/accessor/param').attrib['type'])

            totalstride = sum(stride)

            n_attr = sorted(offsets)[len(offsets) - 1] + 1 
            
            for vertex in range(nv): # iterate through vertices
                for attrib in range(len(offsets)): # iterate though attribs
                    index = indices[vertex * n_attr + offsets[attrib]]
                    match t_attr_str[attrib]:
                        case 'float':
                            for component in range(stride[attrib]): # iterate through each value in the attribute
                                try:
                                    m.write(struct.pack('f', sourcevals[attrib][index * stride[attrib] + component]))
                                except IndexError:
                                    print("ERROR: Index out of range when parsing indices")
                                    print(node.attrib['id'])
                                    print(len(sourcevals), "attribs")
                                    print(len(sourcevals[attrib]), sourcevals[attrib])
                                    print(len(indices), "indices", indices)
                                    print("offsets", offsets) # correct
                                    print("stride", stride) # correct
                                    print("nv", nv, "index", index, "vertex", vertex, "attrib", attrib, "component", component, "stride", stride[attrib])
                                    exit(1)
                        case 'short':
                            pass
                        case 'unsigned byte':
                            pass
                        case 'byte':
                            pass
                        case _:
                            pass

def process(args: argparse.Namespace):
    global file_edit_times
    if os.path.isfile(os.path.join(args.build, 'mdl_edit_times.json')):
        with open(os.path.join(args.build, 'mdl_edit_times.json'), 'r') as times:
            file_edit_times = json.load(times)
    
     
    for root, dirs, models in os.walk(os.path.realpath(args.assets)):
        folder = os.path.relpath(root, os.path.join(args.assets, '../'))
        for modelfile in models:
            if os.path.splitext(modelfile)[1] == '.dae':
                model = os.path.join(root, modelfile)
                if model not in file_edit_times or file_edit_times[model] != os.path.getmtime(model):
                    if not os.path.isdir("romfs/" + folder):
                        os.makedirs("romfs/" + folder)
                    with open(model, 'r') as src:
                        xmlstring = src.read()
                        xmlstring = re.sub(r'\sxmlns="[^"]+"', '', xmlstring, count=1) # remove namespace definition from file
                        mdl = et.fromstring(xmlstring)
                        if args.mirrorpath:
                            convfile(mdl, os.path.join(args.romfs, folder))
                        else:
                            convfile(mdl, args.romfs)
                file_edit_times[model] = os.path.getmtime(model)

    with open(os.path.join(args.build, 'mdl_edit_times.json'), 'w+') as times:
        json.dump(file_edit_times, times)

def add_args(parser: argparse.ArgumentParser):
    parser.add_argument('-b', '--build', required=True, help='Build folder path')
    parser.add_argument('-r', '--romfs', required=True, help='romfs folder path')
    parser.add_argument('-m', '--models', required=True, nargs='+', help='source model path')
    parser.add_argument('-mp', '--mirrorpath', required=False, help='mirror the source path in romfs', action="store_true")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='DAE model parser for SL engine')
    add_args(parser)
    args = parser.parse_args()
    process(args)