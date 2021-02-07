# Try to replace each file in TARGET_DIR subdirs by the same file (name) from SOURCE_DIR subdirs

import os
import shutil

# Update these directories
TARGET_DIR = "nokia5110-screen-stm32f103c8-hal-test"
TARGET_SUBDIRS = ["src"]
SOURCE_DIR = "cubemx-code-generator"
SOURCE_SUBDIRS = ["Core/Inc",
                    "Core/Src"]

# Work from script dir
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
os.chdir(SCRIPT_DIR)

# Checks
print('TARGET_DIR={}'.format(TARGET_DIR))
if(not os.path.isdir(TARGET_DIR)):
    print('\t=> NOT FOUND => ERROR !')
    exit()
print('SOURCE_DIR={}'.format(SOURCE_DIR))
if(not os.path.isdir(SOURCE_DIR)):
    print('\t=> NOT FOUND => ERROR !')
    exit()

print('Trying to replace each file in TARGET_DIR subdirs by the same file (name) from SOURCE_DIR subdirs')
for subdir in TARGET_SUBDIRS:
    dir = os.path.join(TARGET_DIR, subdir)
    for filename in os.listdir(dir):
        # For each file in TARGET_DIR subdirs
        found = False
        for subdir2 in SOURCE_SUBDIRS:
            # Search for replacement in SOURCE_DIR subdirs
            file2 = os.path.join(SOURCE_DIR, subdir2, filename)
            if (os.path.isfile(file2)):
                # Found => copy !
                found = True
                try:
                    shutil.copyfile(file2, os.path.join(dir, filename))
                except shutil.SameFileError:
                    pass
                break
        print('{0:2}'.format('X' if found else ''), end=" ")
        print(filename)
