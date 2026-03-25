import click # pyright: ignore[reportMissingImports]
from os.path import isfile, join
# from SCons.Script import DefaultEnvironment # pyright: ignore[reportMissingImports]
from SCons.Script import ARGUMENTS # pyright: ignore[reportMissingImports]
from SCons.Errors import UserError # pyright: ignore[reportMissingImports]

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

TOOLCHAIN_ROOT = platform.get_package_dir("toolchain-atmelavr")
AVRGGC_DIR = join(TOOLCHAIN_ROOT, "avr-gcc")
AVRGGC_BINDIR = join(AVRGGC_DIR, "bin")

cpp_standard = env.GetProjectOption("cppstd");
c_standard = env.GetProjectOption("cstd");
fastSD = env.GetProjectOption("lib.sd.fast") == "true";
minSD = env.GetProjectOption("lib.sd.min") == "true";
dedicatedSD = env.GetProjectOption("lib.sd.dedicated") == "true";

flags = {
    "CC": [ # C and C++ flags
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
    "CPPDEFINES": [ # Build flags
    ],
}

if fastSD:
    flags['CPPDEFINES'].append(("CHECK_FLASH_PROGRAMMING", 0))

if minSD:
    flags['CPPDEFINES'].append(("SDFAT_FILE_TYPE", 1))
    flags['CPPDEFINES'].append(("USE_FAT_FILE_FLAG_CONTIGUOUS", 0))

if dedicatedSD:
    flags['CPPDEFINES'].append(("ENABLE_DEDICATED_SPI", 1))
else:
    flags['CPPDEFINES'].append(("ENABLE_DEDICATED_SPI", 0))

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

    click.echo("+ SD: ", nl=False)

    if fastSD:
        click.echo("[Fast SD]", nl=False)
    else:
        click.echo("[Safe SD]", nl=False)

    if minSD:
        click.echo("[Minimal SD lib size]", nl=False)
    else:
        click.echo("[Standard SD lib size]", nl=False)

    if dedicatedSD:
        click.echo("[Dedicated SPI]")
    else:
        click.echo("[Shared SPI]")

    click.echo("")
