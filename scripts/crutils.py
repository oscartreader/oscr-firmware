import json
import re
import subprocess
import shutil
import click # pyright: ignore[reportMissingImports]
from datetime import datetime
from time import sleep
from os.path import join, dirname, isfile
from typing import Annotated, NoReturn
from dataclasses import dataclass
from SCons.Script import DefaultEnvironment # pyright: ignore[reportMissingImports]

try:
    from tqdm import tqdm # pyright: ignore[reportMissingModuleSource, reportMissingImports]
except ImportError:
    _env = DefaultEnvironment()
    _env.Execute("$PYTHONEXE -m pip install tqdm")

MYDIR = dirname(__file__)
MYCONF = join(MYDIR, "config.json")

type IfExitCode = Annotated[int, "A value of 0 indicates no error, a value between 1 and 255 indicates an error occurred"] | Annotated[None, "A value of None means return instead of exit"]
type ExitCode = Annotated[int, "A value of 0 indicates no error, a value between 1 and 255 indicates an error occurred"]

def getsig(port, part, proto, speed, env):
    command = [
        env['UPLOADER'],
        "-C",
        env['UPLOADERCONFIG'],
        "-p",
        part,
        "-c",
        proto,
        "-P",
        port,
        "-b",
        "{}".format(speed),
        "-qq",
        "-U",
        "signature:r:-:d"
    ]

    if (proto == "arduino") or (proto == "stk500v1") or (proto == "stk500v2") or (proto == "stk500"):
        command.append("-x")
        command.append("attempts=1")

    result = subprocess.run(command, capture_output=True, text=True)

    if (result.returncode != 0):
        return False
    
    rstr = result.stdout.strip()

    m = re.match("([0-9]+),([0-9]+),([0-9]+)", rstr)

    if not m:
        return False

    return [m.group(1), m.group(2), m.group(3)]

def readid(port, speed, env):
    command = [
        env['UPLOADER'],
        "-C",
        env['UPLOADERCONFIG'],
        "-p",
        "m2560",
        "-c",
        "urclock",
        "-P",
        port,
        "-b",
        "{}".format(speed),
        "-qq",
        "-xid=F.-16.8",
        "-xshowid",
    ]

    result = subprocess.run(command, capture_output=True, text=True)

    if (result.returncode != 0):
        return False
    
    oscr_string = result.stdout.strip()
    oscr_raw = list(reversed([int(oscr_string[i:i + 2], 16) for i in range(0, len(oscr_string), 2)]))

    if (oscr_raw[0] == 0x05) and (oscr_raw[1] == 0xC1) and (oscr_raw[2] == 0x20):
        return {
            "ver": (oscr_raw[3] & 0xF0) >> 4,
            "rev": (oscr_raw[3] & 0x0F),
            "bldrid": oscr_raw[4],
            "slots": oscr_raw[5],
            "hardware": oscr_raw[6],
            "integrated": oscr_raw[7],
        }

    return False

class TqdmBaud(tqdm):
    """Provides a `baud` format parameter"""

    foundBaud: bool = False
    baud: int|str|None = None
    baudFormat: str  = "Testing {baud:>7} baud"
    baudSuccess: str = "Testing {baud:>7} baud succeeded!"
    speedFormat = "({n:>{totalLen}}/{total} speeds)"

    def __init__(self, baud: int|str|None = None, baudFormat: str|None = None, *args, **kwargs):
        self.baud = baud

        if baudFormat:
            self.baudFormat = baudFormat

        if 'bar_format' not in kwargs:
            kwargs['bar_format'] = "{percentage:3.0f}% [{bar}] {baudSpeed} {elapsed_s:5.2f} secs"

        super().__init__(*args, **kwargs)

    def set_baud(self, baud: int|str|None, refresh: bool = True):
        self.baud = baud
        if refresh: self.refresh()

    def success(self):
        self.foundBaud = True
        self.n = self.total
        self.close()

    def failure(self):
        self.foundBaud = False
        self.refresh()
        self.close()

    @property
    def format_dict(self):
        d = super().format_dict

        totalLen = len(str(self.total))
        speedsLen = (totalLen * 2) + 10
        baudSpeedLen = speedsLen + 21

        myDict = {
            "baud": self.baud,
            "n": self.n,
            "total": self.total,
            "totalLen": totalLen,
            "speedsLen": speedsLen,
            "baudSpeedLen": baudSpeedLen,
        }
      
        _baud = "" if self.baud is None else self.baudFormat.format(**myDict)
        baud = "{baud:>{baudLen}}".format(baud=_baud, baudLen=20)

        _speeds = "" if self.foundBaud else self.speedFormat.format(**myDict)
        speeds = "{speeds:>{speedsLen}}".format(speeds=_speeds, **myDict)

        if self.foundBaud:
            _baudSpeed = self.baudSuccess.format(**myDict)
            baudSpeed = "{baudSpeed:<{baudSpeedLen}}".format(baudSpeed=_baudSpeed, **myDict)
        else:
            baudSpeed = "{baud} {speeds}".format(baud=baud, speeds=speeds)

        d.update(baud=baud, speeds=speeds, baudSpeed=baudSpeed)
        return d

def detectbootloaderspeed(port, env, speeds = [250000, 500000, 1000000, 2000000]):
    speedCount = len(speeds)

    pbar = TqdmBaud(total=speedCount, initial=1, miniters=1, mininterval=0, baud=speeds[0])

    for index, speed in enumerate(speeds):
        pbar.set_baud(speed, refresh=False if index == 0 else True)
        sleep(0.1)

        chkdetail = readid(port, speed, env)

        if chkdetail:
            pbar.success()
            sleep(0.5)
            chkdetail['port'] = port
            chkdetail['baud'] = speed
            return chkdetail

        pbar.n += 1

    pbar.failure()

    return False

def isnumber(i):
    return isinstance(i, (int, float, complex)) and not isinstance(i, bool)

def isbool(i):
    return isinstance(i, bool)

def IsEven(num):
    if num % 2 == 0:
        return True
    else:
        return False

def IsOdd(num):
    if num % 2 == 1:
        return True
    else:
        return False

def Bar(char="=", **args):
    bar = char * int(shutil.get_terminal_size().columns);
    click.secho(bar, **args)

def LabeledBar(label, char="=", **args):
    width = len(click.unstyle(label))
    bars = char * int((shutil.get_terminal_size().columns - width - 2) / 2);

    labeledBar = ' '.join([
        bars,
        label,
        bars + (char if IsOdd(width) else "")
    ])

    click.secho(labeledBar, **args)

def PrintError(message: str, header="ERROR"):
    """Prints a generic error. If `code` is `None`, the method returns. Otherwise, it exits with an exit code of `code`."""
    sleep(1)

    click.echo("", err=True)
    LabeledBar(click.style(header, fg="red", bold=True), char="!", err=True)
    click.echo(click.style(message, fg="red", bold=True), err=True)
    Bar(char="!", err=True)
    click.echo("", err=True)

def FatalError(message: str, header="FATAL ERROR", code: ExitCode = 1) -> NoReturn:
    """Prints a fatal error and exits with an exit code of `code`."""
    PrintError(message, header)
    raise SystemExit(0 if code < 0 else (255 if code > 255 else code))

def convifbool(v):
    if (isinstance(v, bool)):
        return "true" if v else "false"
    return v

def askYNQuestion(question):
    userAnswer = None
    while userAnswer == None:
        print(question, " [(Y)es/(n)o]")
        requestInput = input().lower()

        if requestInput == "" or requestInput == "y" or requestInput == "yes" or requestInput == "true":
            userAnswer = True
        elif requestInput == "n" or requestInput == "no" or requestInput == "false":
            userAnswer = False
        else:
            print("Invalid response. Type Y or N and then press enter/return.")
            print("")

    return userAnswer

class CRSettings:
    file = join(MYDIR, "settings.json")
    section = None
    settings = {}

    def __init__(self, section):
        self.section = section
        self.reload()

    def reload(self):
        if isfile(self.file):
            with open(self.file, 'r', encoding='UTF-8') as f:
                self.settings = json.load(f)
        else:
            self.settings = {}

        if self.settings.setdefault(self.section, {}) == {}:
            now = datetime.now()
            self.settings[self.section]['_created'] = now.strftime("%Y-%m-%d %H-%M-%S")
            self.settings[self.section]['_created_ts'] = now.timestamp()
            self.save()

    def get(self, setting, default = None):
        return self.settings.get(self.section, {}).get(setting, default)

    def set(self, setting, value):
        self.settings[self.section][setting] = value

    def save(self):
        now = datetime.now()
        self.settings[self.section]['_modified'] = now.strftime("%Y-%m-%d %H-%M-%S")
        self.settings[self.section]['_modified_ts'] = now.timestamp()
        with open(self.file, 'w') as f:
            json.dump(self.settings, f)
        self.reload()

class CRConfig:
    env = None
    envName = None
    config = {}
    platformConf = {}
    boardConf = {}
    sections = [
        "oscr",
        "options",
        "cores"
    ],
    overrides = {
        "oscr": "oscr",
        "options": "option",
        "cores": "core",
        "hardware": "hardware",
        "output": "output",
        "input": "input"
    }
    __target = {
        "board": None,
        "arch": None,
    }

    def __init__(self, env):
        self.env = env

        self.platformConf = env.PioPlatform()
        self.boardConf = env.BoardConfig()
        self.projConf = env.GetProjectConfig()

        self.envName = env.get("PIOENV")

        tmpArch = self.platformConf.name

        match tmpArch:
            case "atmelavr": self.__target['arch'] = "avr"
            case _: self.__target['arch'] = None

        del tmpArch

        self.__target['board'] = self.boardConf.get("build", {}).get("mcu")

        with open(MYCONF) as f:
            self.config = json.load(f)

        hardwareConf = self.config.get("flags").get("hardware")
        outputConf = self.config.get("flags").get("output")
        inputConf = self.config.get("flags").get("input")
        featuresConf = self.config.get("flags").get("features")
        optionsConf = self.config.get("flags").get("options")
        coreConf = self.config.get("cores")

        self.__hwoptions = CRHardwareOptions(self, hardwareConf)
        self.__output = CROutput(self, outputConf)
        self.__input = CRInput(self, inputConf)
        self.__features = CRFeatures(self, featuresConf)
        self.__options = CROptions(self, optionsConf)
        self.__cores = CRCores(self, coreConf)

    @property
    def arch(self):
        return self.__target['arch']

    @property
    def board(self):
        return self.__target['board']

    @property
    def target(self):
        return self.getTarget()

    @property
    def cores(self):
        return self.__cores

    @property
    def features(self):
        return self.__features

    @property
    def options(self):
        return self.__options

    @property
    def output(self):
        return self.__output

    @property
    def input(self):
        return self.__input

    @property
    def hardware(self):
        return self.__hwoptions

    @property
    def projectOptions(self):
        return self.projConf.options(env=self.envName)

    def getSectionValues(self, section):
        options = self.projConf.items(section, as_dict=True)

        overrideKey = self.overrides.get(section, None)

        if overrideKey == None:
            return options

        overrides = [
            self.projConf.items(section="oscr", as_dict=True),
            self.projConf.items(env=self.envName, as_dict=True)
        ]

        for overrideOptions in overrides:
            for key, value in overrideOptions.items():
                if "." not in key or not key.startswith(overrideKey + "."):
                    continue

                (keyType, keyFor) = key.split('.', 1)
                options[keyFor] = value

        return options

    def getArch(self):
        return self.__target['arch']

    def getBoard(self):
        return self.__target['board']

    def getTarget(self):
        if ((self.arch != None) and (self.board != None)):
            target = ".".join([self.arch, self.board])
        elif (self.board != None):
            target = self.board
        elif (self.arch != None):
            target = self.arch
        else:
            target = "unknown"

        return target

    def getCoreFlags(self):
        return self.__cores.getFlags()

    def getHardwareFlags(self):
        return self.__hwoptions.getFlags()

    def getOutputFlags(self):
        return self.__output.getFlags()

    def getInputFlags(self):
        return self.__input.getFlags()

    def getFeatureFlags(self):
        return self.__features.getFlags()

    def getOptionFlags(self):
        return self.__options.getFlags()

class CRFeature:
    name = None
    define = None
    __type = None
    __raw = ""
    __value = False

    def __init__(self, target, data, value):
        self.target = target
        self.name = data.get("name")
        self.define = data.get("define", None)

        self.__type = data.get("type", "ifdef")
        self.__raw = value

        match self.__type:
            #
            # ifdef
            #
            case "ifdef":
                if (value == True):
                    self.__value = True
                elif (value == False) or (value == None):
                    self.__value = False
                elif (value.lower() == "true"):
                    self.__value = True
                elif (value.lower() == "false"):
                    self.__value = False
                else:
                    ifdef = data.get("ifdef")

                    self.__value = False

                    for k, v in ifdef.items():
                        if (value == k):
                            self.__value = v['value']
                            break
            #
            # def
            #
            case "def":
                defvals = data.get("def", {})
                defval = defvals.get(value.lower(), {}).get("values")

                self.__value = defval
            #
            # bool
            #
            case "bool":
                if (value == True):
                    self.__value = True
                elif (value == False) or (value == None):
                    self.__value = False
                elif (value.lower() == "true"):
                    self.__value = True
                elif (value.lower() == "false"):
                    self.__value = False
                else:
                    boolvals = data.get("bool")

                    self.__value = False

                    for k, v in boolvals.items():
                        if (self.__value == k):
                            self.__value = v['value']
                            break
            #
            # enum
            #
            case "enum":
                enumEntry = data.get("enum", {}).get(value.lower())
                if (enumEntry != None):
                    self.__value = enumEntry.get("value", None)
                else:
                    self.__value = None
            #
            # bitflag
            #
            case "bitflag" if value == "":
                self.__value = []

            case "bitflag":
                bitflags = [v.strip() for v in value.lower().split('|')]

                flags = []

                for bitflag in bitflags:
                    opt = data.get("bitflags", {}).get(bitflag, None)
                    optFlag = opt.get("flag")
                    flags.append(optFlag)

                self.__value = flags
            #
            # int
            #
            case "int":
                intRange = data.get("int", {})

                if not isnumber(value):
                    value = int(value)

                if intRange.get('min') and value < intRange.get('min'):
                    raise ValueError("Error [{}]: Value too low.".format(self.name))

                if intRange.get('max') and value > intRange.get('max'):
                    raise ValueError("Error [{}]: Value too high.".format(self.name))

                self.__value = value
            case _:
                print("Unknown option:" + self.__type)

    @property
    def enabled(self):
        if (self.type == "ifdef") or (self.type == "bool"):
            return self.__value
        else: # Not a toggle
            return None

    @property
    def value(self):
        if (self.type == "ifdef"):
            return self.define if self.__value == True else None
        elif (self.type == "bool"):
            return (self.define, self.__value)
        elif (self.type == "int"):
            return (self.define, self.__value)
        elif (self.type == "bitflag"):
            return None if len(self.__value) < 1 else "\"" + ("=".join([self.define, '|'.join(self.__value)])) + "\""
        elif (self.type == "enum"):
            return (self.define, self.__value) if self.__value != None else None
        else:
            return self.__value

    @property
    def type(self):
        return self.__type

class CROptionBase:
    sectionKey = ""

    def __init__(self, sectionKey, cr, conf):
        self.sectionKey = sectionKey
        self.cr = cr
        self.conf = conf

        configs = self.cr.getSectionValues(self.sectionKey)

        for key, conf in configs.items():
            option = self.conf.get(key)

            if (option == None) or (conf == None) or (("define" not in option) and ("type" not in option)):
                continue

            self._flags[key] = CRFeature(self.cr.target, option, conf)

    def getFlags(self):
        flags = []

        for feature in self._flags.values():
            match feature.type:
                #
                # ifdef
                #
                case "ifdef":
                    if not feature.enabled: # skip if not enabled
                        continue
                    if isinstance(feature.value, str): # just append strings
                        flags.append(feature.value)
                        continue
                    for v in feature.value: # append each value from lists
                        flags.append(v)
                #
                # def
                #
                case "def":
                    if (feature.value == None): # skip if not enabled
                        continue
                    if isinstance(feature.value, str): # just append strings
                        flags.append(feature.value)
                        continue
                    for k, v in feature.value.items(): # append each value from lists
                        flags.append((k, convifbool(v)))
                case "bool":
                    flags.append((feature.define, convifbool(feature.enabled)))
                #
                # everything else
                #
                case _:
                    if(feature.value != None):
                        flags.append(feature.value)

        return flags

class CRFeatures(CROptionBase):
    _flags = {}

    def __init__(self, cr, conf):
        super().__init__("oscr", cr, conf)

class CROptions(CROptionBase):
    _flags = {}

    def __init__(self, cr, conf):
        super().__init__("options", cr, conf)

class CRHardwareOptions(CROptionBase):
    _flags = {}

    def __init__(self, cr, conf):
        super().__init__("hardware", cr, conf)

class CROutput(CROptionBase):
    _flags = {}

    def __init__(self, cr, conf):
        super().__init__("output", cr, conf)

class CRInput(CROptionBase):
    _flags = {}

    def __init__(self, cr, conf):
        super().__init__("input", cr, conf)

class CRCore:
    name = None
    define = None
    crdb = []
    auto = []
    __enabled = False

    def __init__(self, target, data, enabled):
        self.target = target
        self.name = data.get("name")
        self.define = data.get("define")
        self.crdb = data.get("crdb", [])
        self.auto = data.get("auto", [])

        if (enabled.lower() == "true"):
            self.__enabled = True
        elif (enabled.lower() == "false"):
            self.__enabled = False
        elif (enabled.lower() == "auto"):
            if (self.target in self.auto):
                self.__enabled = True
            else:
                self.__enabled = False

    @property
    def enabled(self):
        return self.__enabled

    @property
    def value(self):
        return self.define if self.__value == True else None


class CRCores:
    env = None
    cr = {}
    conf = {}
    cores = {}

    def __init__(self, cr, conf):
        self.cr = cr
        self.conf = conf

        optionKeys = self.cr.getSectionValues("cores")

        for key, conf in optionKeys.items():
            core = self.conf.get(key)

            if (core == None) or (conf == None) or ("define" not in core):
                continue

            self.cores[key] = CRCore(self.cr.target, core, conf)

    def getFlags(self):
        flags = []

        for core in self.cores.values():
            if (core.enabled): flags.append(core.define)

        return flags
