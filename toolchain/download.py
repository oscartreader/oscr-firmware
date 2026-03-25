import json, shutil
import click # pyright: ignore[reportMissingImports]
from os import getcwd, rename, unlink
from os.path import join, isdir, isfile
from time import sleep
from platformio import util # pyright: ignore[reportMissingImports]
from platformio.package.lockfile import LockFile # pyright: ignore[reportMissingImports]
from platformio.package.download import FileDownloader # pyright: ignore[reportMissingImports]
from platformio.package.unpack import FileUnpacker # pyright: ignore[reportMissingImports]
from platformio.package.exception import PackageException # pyright: ignore[reportMissingImports]
from SCons.Script import ARGUMENTS # pyright: ignore[reportMissingImports]
import tcutils

Import("env") # pyright: ignore[reportUndefinedVariable]
if not env: # pyright: ignore[reportUndefinedVariable]
  env = {}
  click.echo("Internal error: No env", err=True)
  exit(1)

VERBOSE = int(ARGUMENTS.get("PIOVERBOSE", 0))

PROJECT_ROOT = getcwd()
TOOLCHAIN_ROOT = tcutils.Dir()
CONFIG_FILE = join(TOOLCHAIN_ROOT, "config.json")
AVRGGC_DIR = join(TOOLCHAIN_ROOT, "avr-gcc")
AVRGGC_VERFILE = join(AVRGGC_DIR, "version.txt")

updating = None
currentVersion = None

def main():
    archive = getArchiveData()

    AVRGGC_ARCHIVE = join(TOOLCHAIN_ROOT, archive['filename'])
    ARCHIVE_DIR = join(TOOLCHAIN_ROOT, archive['directory'])

    click.echo("")

    if (isUpdating()):
        if (archive == None):
            click.echo("Unsupported platform cannot be updated automatically.")
            return
        if (currentVersion == archive['version']):
            click.echo("Toolchain already installed and correct version.")
            return

    with LockFile(TOOLCHAIN_ROOT):
        #
        # Download
        #
        fd = FileDownloader(archive['url'])
        fd.set_destination(AVRGGC_ARCHIVE)

        if isfile(AVRGGC_ARCHIVE):
            click.echo("Verifying existing archive ... ", nl=False)

            try:
                fd.verify(archive['checksum'])
                click.echo("OK")
            except PackageException:
                click.echo("FAILED")

                click.echo("Removing archive ... ", nl=False)
                unlink(AVRGGC_ARCHIVE)
                click.echo("OK")

        if not isfile(AVRGGC_ARCHIVE):
            click.echo("Downloading archive ... ", nl=False)

            fd.start(with_progress=False, silent=True)

            if not isfile(AVRGGC_ARCHIVE):
                click.echo("FAILED")
                tcutils.GenericError("Failed to download archive.")

            click.echo("OK")

        #
        # Verify
        #
        click.echo("Verifying archive ... ", nl=False)

        fd.verify(archive['checksum'])

        click.echo("OK")

        #
        # Extract
        #
        click.echo("Extracting archive ... ", nl=False)

        with FileUnpacker(AVRGGC_ARCHIVE) as fu:
            assert fu.unpack(TOOLCHAIN_ROOT, with_progress=False, silent=True);

        if not isdir(ARCHIVE_DIR):
            click.echo("FAILED")
            tcutils.GenericError("Expected `"+ARCHIVE_DIR+"` to exist and be a directory.")

        click.echo("OK")

        if updating and isdir(AVRGGC_DIR):
            if (AVRGGC_DIR == TOOLCHAIN_ROOT) or (AVRGGC_DIR == PROJECT_ROOT): # Safety
                tcutils.ConfigError("Script Error: This should not happen.")
            click.echo("Removing previous version...")
            shutil.rmtree(AVRGGC_DIR)
            click.echo("")

        click.echo("Installing ... ", nl=False)

        rename(ARCHIVE_DIR, AVRGGC_DIR)

        if not isdir(AVRGGC_DIR):
            click.echo("FAILED")
            tcutils.GenericError("Expected `"+AVRGGC_DIR+"` to exist and be a directory.")

        with open(AVRGGC_VERFILE, "w") as f:
            f.write(archive['version'])

        click.echo("OK")

        click.echo("")
        click.echo("Toolchain installed successfully.")

        sleep(2)

def isUpdating():
    global updating, currentVersion

    if (updating != None): return updating

    if not isdir(AVRGGC_DIR):
        updating = False
    else:
        updating = True

        if not isfile(AVRGGC_VERFILE):
            tcutils.GenericError("Could not determine the current version.")

        with open(AVRGGC_VERFILE) as f:
            currentVersion = f.read()

    return updating

def getSysType():
    sysType = util.get_systype()

    match sysType:
        case "windows_amd64":
            configSysKey = "win64"

        case "windows" | "windows_x86":
            configSysKey = "win32"

        case "windows_arm64":
            configSysKey = "winARM"

        case "linux_x86_64":
            configSysKey = "lin64"

        case "linux_armv7l":
            configSysKey = "linARM"

        case "darwin_arm64":
            configSysKey = "macARM"

        case _:
            configSysKey = None

    return configSysKey

def getArchiveData():
    sysType = getSysType()

    if (sysType == None):
        if not isUpdating(): tcutils.HostError("Unknown host platform/OS")
        else:
            tcutils.HostWarning("Unknown host platform/OS")
            return None

    with open(CONFIG_FILE) as f:
        config = json.load(f)

        if sysType not in config['archives']:
            tcutils.HostError("Unsupported host platform/OS")

        archive = config['archives'][sysType]

        if ('filename' not in archive) or ('url' not in archive) or ('directory' not in archive) or ('checksum' not in archive):
            tcutils.ConfigError("Config Error: Archive data is misconfigured/malformed.")

        if "avr-gcc" not in config['versions']:
            tcutils.ConfigError("Config Error: Could not determine AVR GCC version.")
        else:
            archive['version'] = config['versions']['avr-gcc']

        return archive

if (__name__ == "SCons.Script"):
    main()

click.echo("")
