#!/usr/bin/python3

import sys
import traceback

def main(warbleHome, argv):
    try:
        sys.path.append(warbleHome)
        sys.argv = [ "warblecc.py" ]
        for item in eval(argv):
            sys.argv.append(item)
        import warblecc
        warblecc.main()
    except Exception as e:
        print(e)
        print(traceback.format_exc())
