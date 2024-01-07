import argparse
import daetomdl, copyfiles

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Build tasks script')
    
    daetomdl.add_args(parser)
    copyfiles.add_args(parser)

    args = parser.parse_args()

    daetomdl.process(args)
    copyfiles.process(args)
