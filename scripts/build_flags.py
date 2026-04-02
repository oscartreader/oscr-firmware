import click # pyright: ignore[reportMissingImports]
from os import getcwd
from os.path import join, isfile
from SCons.Script import ARGUMENTS # pyright: ignore[reportMissingImports]
from SCons.Errors import UserError # pyright: ignore[reportMissingImports]
import crutils

envs = []

VERBOSE = int(ARGUMENTS.get("PIOVERBOSE", 0))

try:
    Import("env") # pyright: ignore[reportUndefinedVariable]
    envs.append(env); # pyright: ignore[reportUndefinedVariable]
except (NameError, KeyError, UserError):
    env = None

if not env: # pyright: ignore[reportUndefinedVariable]
    env = {}
    print("Internal error: env missing")
    exit(1)

try:
    Import("projenv") # pyright: ignore[reportUndefinedVariable]
    envs.append(projenv); # pyright: ignore[reportUndefinedVariable]
except (NameError, KeyError, UserError):
    projenv = None

envglobal = DefaultEnvironment() # pyright: ignore[reportUndefinedVariable]

envs.append(envglobal)

platform = env.PioPlatform()

coreConfig = crutils.CRConfig(env)

confDefines = coreConfig.getCoreFlags() + coreConfig.getHardwareFlags() + coreConfig.getFeatureFlags() + coreConfig.getOutputFlags() + coreConfig.getInputFlags() + coreConfig.getOptionFlags()

TOOLCHAIN_ROOT = platform.get_package_dir("toolchain-atmelavr")
AVRGGC_DIR = join(TOOLCHAIN_ROOT, "avr-gcc")
AVRGGC_BINDIR = join(AVRGGC_DIR, "bin")

#
# Create user.ini if it doesn't exist.
#

PROJECT_ROOT = getcwd()

userFile = join(PROJECT_ROOT, "user.ini")
userFileNew = join(PROJECT_ROOT, ".user.new.ini")

if not isfile(userFile) and isfile(userFileNew):
    with open(userFileNew, 'r', encoding='UTF-8') as srcFile, open(userFile, 'w', encoding='UTF-8') as destFile:
        while line := srcFile.readline():
            if not line.startswith(";@"):
                destFile.write(line)

#
# End create user.ini
#

cpp_standard = env.GetProjectOption("cppstd");
c_standard = env.GetProjectOption("cstd");

flags = {
    "CC": [ # C and C++ flags
        "-Wall",
        "-Werror",
        "-flto=auto",
        "-fuse-linker-plugin",
    ],
    "C": [ # C flags
        "--language=c",
        "-std=" + c_standard,
    ],
    "CXX": [ # C++ flags
        "--language=c++",
        "-std=" + cpp_standard,
        "-fpermissive",
        "-fno-threadsafe-statics",
        "-Wno-volatile",
    ],
    "LINKER": [ # Linker flags
        "-Wall",
        "-Wextra",
        "-Os",
        "-g",
        "-flto=auto",
        "-fuse-linker-plugin",
        "-Wl,--gc-sections",
        "-Wl,--relax",
    ],
    "CPPDEFINES": confDefines, # Build flags
}

for _env in envs:
    _env.AppendUnique(CCFLAGS=flags['CC'])
    _env.AppendUnique(CFLAGS=flags['C'])
    _env.AppendUnique(CXXFLAGS=flags['CXX'])
    _env.AppendUnique(LINKFLAGS=flags['LINKER'])
    _env.AppendUnique(CPPDEFINES=flags['CPPDEFINES'])

    _env.Replace(
        UPLOADER=join(AVRGGC_BINDIR, "avrdude"),
        UPLOADERFLAGS=[
            "-p",
            "$BOARD_MCU",
            "-C",
            join(AVRGGC_BINDIR, "avrdude.conf"),
            "-c",
            "$UPLOAD_PROTOCOL",
        ],
        UPLOADCMD="$UPLOADER $UPLOADERFLAGS -U flash:w:$SOURCES:i",
        UPLOADEEPCMD="$UPLOADER $UPLOADERFLAGS -U eeprom:w:$SOURCES:i",
    )

    _env.PrependENVPath(
        "PATH",
        AVRGGC_BINDIR,
    )

if VERBOSE:
    click.echo("")
    click.echo("+ C flags: " + env['CFLAGS'])
    click.echo("+ CXX flags: " + env['CXXFLAGS'])
    click.echo("+ Linker flags: " + env['LINKFLAGS'])
    click.echo("")
    click.echo("+ Defines")
    for define in confDefines:
        if (isinstance(define, str)):
            click.echo("  - {}".format(define))
        else:
            click.echo("  - {}: {}".format(define[0], define[1]))
    click.echo("")
