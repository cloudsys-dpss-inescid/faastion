#!/usr/bin/python3
import os
import sys

import run_test
from run_utils import print_message, MessageType
from run_globals import TEST_CONFIG_DIR, MAX_VERBOSITY_LVL

# Global variables.
VERBOSITY_LVL = 0


def set_verbosity(flag):
    global VERBOSITY_LVL
    v_count = flag.count("v")
    if v_count != len(flag):
        print_message("Verbosity flag should be v or vv instead of {flag}. Output verbosity level will fallback to 0."
                      .format(flag=flag), MessageType.WARN)
        return
    VERBOSITY_LVL = min(v_count, MAX_VERBOSITY_LVL)
    print_message("Output verbosity level is set to {level}.".format(level=VERBOSITY_LVL), MessageType.SPEC)


def preprocess_filters(include_filter):
    tier_filters, language_filters = [], []
    for filter in include_filter.split(","):
        (language_filters, tier_filters)[filter.find("tier-") >= 0].append(filter)
    return tier_filters, language_filters


def should_filter_dir(directory, tier_filters, language_filters):
    # Is directory equals to testing root directory?
    if directory == TEST_CONFIG_DIR:
        return True

    # If we doesn't have any to filter, return.
    if len(tier_filters) == 0 and len(language_filters) == 0:
        return False

    # Check tier filters first.
    for filter in tier_filters:
        if directory.find(filter) >= 0:
            break
    else:
        # In cases when we finish with iterating without break.
        if len(tier_filters) > 0:
            return True

    # Check language filters.
    for filter in language_filters:
        index = directory.find(filter)
        # We need to find filter and distinguish between java and javascript cases.
        if index >= 0 and index + len(filter) == len(directory):
            return False
    else:
        # In cases when we finish with iterating without break.
        return len(language_filters) > 0


def should_filter_file(file):
    return True if file == ".gitkeep" else False


def test_all(include_filter):
    tier_filters, language_filters = preprocess_filters(include_filter)
    for root, dirs, files in sorted(os.walk(TEST_CONFIG_DIR)):
        # Do a filtration based on users requirements or if we are not in leaf directory.
        if should_filter_dir(root, tier_filters, language_filters) or len(dirs) > 0:
            continue
        root_split = root.split(os.path.sep)
        current_tier = os.path.join(root_split[-2], root_split[-1])
        print_message("{}...".format(current_tier), MessageType.INFO)
        for file in sorted(files):
            if should_filter_file(file):
                continue
            print_message("Test - {} - is running...".format(file.split(".json")[0]), MessageType.SPEC)
            run_test.main(["v" * VERBOSITY_LVL, os.path.join(root, file)])
            print_message("Test - {} - is running...done".format(file.split(".json")[0]), MessageType.SPEC)
        print_message("{}...done".format(current_tier), MessageType.INFO)


# Main function.
def main(args):
    include_filter = ""
    if len(args) == 0:
        print_message("Output verbosity level will be 0.", MessageType.SPEC)
    elif len(args) == 1:
        set_verbosity(args[0])
    elif len(args) == 2:
        set_verbosity(args[0])
        include_filter = args[1]
    else:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)
    test_all(include_filter)


if __name__ == '__main__':
    main(sys.argv[1:])
