import json, shutil
import click # pyright: ignore[reportMissingImports]
from time import sleep
from os.path import join, dirname

MYDIR = dirname(__file__)
MYCONF = join(MYDIR, "config.json")

def isnumber(i):
    return isinstance(i, (int, float, complex)) and not isinstance(i, bool)

def convifbool(v):
    if (isinstance(v, bool)):
        return "true" if v else "false"
    return v

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

                #print("for {}, override {} with {}".format(section, keyFor, value))

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
            return "\"" + ("=".join([self.define, '|'.join(self.__value)])) + "\""
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

        #print("Found {} {}".format(len(self._flags), sectionKey))

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

        #optionKeys = self.cr.projConf.options("cores")
        optionKeys = self.cr.getSectionValues("cores")

        for key, conf in optionKeys.items():
            core = self.conf.get(key)
            #conf = self.cr.projConf.get("cores", key)

            if (core == None) or (conf == None) or ("define" not in core):
                continue

            self.cores[key] = CRCore(self.cr.target, core, conf)

        #print("Found {} cores".format(len(self.cores)))

    def getFlags(self):
        flags = []

        for core in self.cores.values():
            if (core.enabled): flags.append(core.define)

        return flags
