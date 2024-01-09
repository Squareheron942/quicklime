import argparse, os, shutil, re, json, ffmpeg

scripts = []
materials = []

filter_arg_to_n = {
    "NEAREST": 0,
    "LINEAR": 1
}

wrap_arg_to_n = {
    "CLAMP_EDGE": 0,
    "CLAMP_BORDER": 1,
    "REPEAT": 2,
    "MIRRORED_REPEAT": 3
}

def find_base_class(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # Define a regular expression pattern to find class declarations and inheritance
    pattern = r'class\s+(\w+)\s*(:\s*public\s+(\w+))?'
    matches = re.finditer(pattern, content)

    main_class = None
    base_class = None

    for match in matches:
        class_name = match.group(1)
        extends_class = match.group(3)

        if not main_class:
            main_class = class_name

        if extends_class and class_name == main_class:
            base_class = extends_class
            break

    return main_class, base_class

def run_fast_scandir(dir, ext):    # dir: str, ext: list
    subfolders, files = [], []

    for f in os.scandir(dir):
        if f.is_dir():
            subfolders.append(f.path)
        if f.is_file():
            if os.path.splitext(f.name)[1].lower() in ext:
                files.append(f.path)


    for dir in list(subfolders):
        sf, f = run_fast_scandir(dir, ext)
        subfolders.extend(sf)
        files.extend(f)
    return subfolders, files

def process(args: argparse.Namespace):
    # copy all scene files to romfs
    subfolders, files = run_fast_scandir(args.assets, [".scene"])
    for file in files:
        shutil.copy(file, os.path.join(args.romfs, 'scenes'))

    # copy all headers to include dir
    subfolders, files = run_fast_scandir(args.assets, [".h", ".inc", ".hpp"])
    for file in files:
        shutil.copy(file, args.include)
        main_class, base_class = find_base_class(file)
        print(main_class + " header")
        if base_class == "Script":
            scripts.append(os.path.basename(file))
        elif base_class == "material":
            materials.append(os.path.basename(file))

    # compile all user defined scripts into one header
    with open(os.path.join(args.include, "scripts.inc"), 'w+') as scriptheader:
        scriptheader.write('#pragma once\n')
        scriptheader.writelines(('#include "' + s + '"\n') for s in scripts)
    
    # compile all user defined materials into one header
    with open(os.path.join(args.include, "materials.inc"), 'w+') as scriptheader:
        scriptheader.write('#pragma once\n')
        scriptheader.writelines(('#include "' + s + '"\n') for s in materials)

    # copy all user source code files to the correct source code directory
    subfolders, files = run_fast_scandir(args.assets, [".c", ".cpp", ".pica"]) # copy all source code files to the temporary source code folders
    for file in files:
        shutil.copy(file, args.source)

    # copy all image files to gfx folder and add t3s files from the config files (+ create the romfs config file)
    subfolder, files = run_fast_scandir(args.assets, [".png"])
    for file in files:
        if os.path.exists(os.path.splitext(file)[0] + '.texinf'):
            with open(os.path.splitext(file)[0] + '.texinf', 'r') as config:
                info = json.load(config)
                size = 128
                vram = False
                compression = "auto"
                format = "auto-etc1"
                wrap = {}
                wrap['S'] = "REPEAT"
                wrap['T'] = "REPEAT"
                filter = {}
                filter["min"] = "NEAREST"
                filter["mag"] = "LINEAR"

                try:
                    format = info['format']
                except KeyError:
                    pass
                try:
                    size = info['size']
                except KeyError:
                    pass
                try:
                    vram = info['vram']
                except KeyError:
                    pass
                try:
                    compression = info['compression']
                except KeyError:
                    pass
                try:
                    wrap = info['wrap']
                except KeyError:
                    pass
                try:
                    filter = info['filter']
                except KeyError:
                    pass

                arg = 0

                if (vram):
                    arg |= 1 << 7
                arg |= filter_arg_to_n[filter['mag']] << 6
                arg |= filter_arg_to_n[filter['min']] << 5
                arg |= wrap_arg_to_n[wrap['S']] << 3
                arg |= wrap_arg_to_n[wrap['T']] << 1

                with open(os.path.join(args.gfx, os.path.splitext(os.path.basename(file))[0]) + '.t3s', 'w+') as t3s: 
                    t3s.write(f'-f {format} -z {compression} ')
                    t3s.write(os.path.basename(file)) # file will be copied into here so it's fine
                    print(file)
                    stream = ffmpeg.input(file)
                    stream = ffmpeg.filter(stream, "scale", size, size)
                    stream = ffmpeg.output(stream, os.path.join(args.gfx, os.path.basename(file))) 
                    ffmpeg.run(stream, overwrite_output=True, quiet=True)
                    # TODO replace hardcoded path with variable
                    with open(os.path.join(args.romfs, "gfx", os.path.splitext(os.path.basename(file))[0] + '.t3xcfg'), 'wb+') as t3xcfg:
                        t3xcfg.write((arg).to_bytes(1))


def add_args(parser: argparse.ArgumentParser):
    try:
        parser.add_argument('-a', '--assets', required=True, help='asset files path')
    except argparse.ArgumentError:
        pass
    try:
        parser.add_argument('-b', '--build', required=True, help='Build folder path')
    except argparse.ArgumentError:
        pass
    try:
        parser.add_argument('-s', '--source', required=True, help='destination source code path')
    except argparse.ArgumentError:
        pass
    try:
        parser.add_argument('-i', '--include', required=True, help='destination include path')
    except argparse.ArgumentError:
        pass
    try:
        parser.add_argument('-g', '--gfx', required=True, help='destination graphics file path')
    except argparse.ArgumentError:
        pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Copy files to correct locations from asset folder')
    add_args(parser)
    args = parser.parse_args()
    process(args)