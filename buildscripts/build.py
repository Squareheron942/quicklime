import argparse, daetomdl

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Build tasks script')
    parser.add_argument('-r', '--romfs', required=True, help='romfs folder path')
    parser.add_argument('-m', '--models', required=True, nargs='+', help='source model path')
    parser.add_argument('-mp', '--mirrorpath', required=False, help='mirror the source path in romfs', action="store_true")
    args = parser.parse_args()
    daetomdl.process(args)