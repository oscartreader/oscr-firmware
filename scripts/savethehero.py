import json, crutils

from os import getcwd
from os.path import join, isdir, isfile
from SCons.Script import ( # pyright: ignore[reportMissingImports]
    DefaultEnvironment,
    ARGUMENTS
)
from SCons.Errors import UserError # pyright: ignore[reportMissingImports]

VERBOSE = int(ARGUMENTS.get("PIOVERBOSE", 0))

Import("env")  # pyright: ignore[reportUndefinedVariable]

if not env: # pyright: ignore[reportUndefinedVariable]
    env = {}
    print("Internal error: env missing")
    exit(1)

try:
    Import("projenv") # pyright: ignore[reportUndefinedVariable]
    envs.append(projenv); # pyright: ignore[reportUndefinedVariable]
except (NameError, KeyError, UserError):
    projenv = None

MySettings = crutils.CRSettings("savethehero")

hasRTC = None
hasClockGenerator = None

config = env.GetProjectConfig()

def loadSettings():
    global hasRTC, hasClockGenerator, MySettings
    hasRTC = MySettings.get('has-rtc', None)
    hasClockGenerator = MySettings.get('has-clockgen', None)

def importSettings():
    global hasRTC, hasClockGenerator, config

    loadSettings()

    if hasRTC == True:
        config.set("hardware", "rtc", True)
    elif hasRTC == False:
        config.set("hardware", "rtc", False)

    if hasClockGenerator == True:
        config.set("hardware", "clockgen", True)
    elif hasClockGenerator == False:
        config.set("hardware", "clockgen", False)


def checkConfig():
    global hasRTC, hasClockGenerator

    loadSettings()

    if VERBOSE:
        print('[SaveTheHero]')

    if hasRTC == None or hasClockGenerator == None:
        if VERBOSE:
            print("User has not configured their hardware, asking for configuration...")
        setupSTH()
    else:
        if VERBOSE:
            print('Using existing configuration: ')
            print('- Clock Generator:', "Yes" if hasClockGenerator else "No")
            print('- RTC:', "Yes" if hasRTC else "No")
            print('')

    importSettings()


def setupSTH(source = None, target = None, env = None):
    global hasRTC, hasClockGenerator

    print("")
    print("")

    if source != None:
        loadSettings()

        print("[SaveTheHero] Current Settings")
        print('- Clock Generator:', "Yes" if hasClockGenerator else "No")
        print('- RTC:', "Yes" if hasRTC else "No")
        print("")

    hasRTC = crutils.askYNQuestion("Does your cartridge reader have the real-time clock (RTC) upgrade?")

    if hasRTC:
        hasClockGenerator = True
    else:
        hasClockGenerator = crutils.askYNQuestion("Does your cartridge reader have the clock generator addon?")

    MySettings.set('has-clockgen', hasClockGenerator)
    MySettings.set('has-rtc', hasRTC)
    MySettings.save()

    if source != None:
        loadSettings()

        print("[SaveTheHero] Updated Settings")
        print('- Clock Generator:', "Yes" if hasClockGenerator else "No")
        print('- RTC:', "Yes" if hasRTC else "No")
        print("")

env.AddTarget(
    name="savethehero-settings",
    dependencies=None,
    actions=[
        setupSTH,
    ],
    title="ClockGen/RTC Settings",
    description="Change the settings related to if you have a clock generator and/or real-time clock (RTC).",
    always_build=True,
    group="General"
)

hasRTC = MySettings.get('has-rtc', None)
hasClockGenerator = MySettings.get('has-clockgen', None)

# Override settings if we have them

importSettings()

if env.IsIntegrationDump():
    # stop the current script execution
    Return() # pyright: ignore[reportUndefinedVariable]

checkConfig()
