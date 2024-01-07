import argparse, os, shutil

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

    subfolders, files = run_fast_scandir(args.assets, [".h", ".inc", ".hpp"])
    for file in files:
        shutil.copy(file, args.include)

    subfolders, files = run_fast_scandir(args.assets, [".c", ".cpp", ".pica"]) # copy all source code files to the temporary source code folders
    for file in files:
        shutil.copy(file, args.source)

    # copy all image files to gfx folder and add t3s files from the config files (+ create the romfs config file)
    subfolder, files = run_fast_scandir(args.assets, [".png"])
    for file in files:
        if os.path.exists(os.path.splitext(file)[0] + '.t3xcfg'):
            with open(os.path.splitext(file)[0] + '.t3xcfg', 'rb') as config:
                with open(os.path.join(args.gfx, os.path.splitext(os.path.basename(file))[0]) + '.t3s', 'w+') as t3s: 
                    t3s.write('-f auto-etc1 -z auto ')
                    t3s.write(os.path.abspath(file))
                    print(file)
                    pass


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