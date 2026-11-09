import click # pyright: ignore[reportMissingImports]
import crutils
from time import sleep
from SCons.Errors import UserError # pyright: ignore[reportMissingImports]

try:
  Import("env") # pyright: ignore[reportUndefinedVariable]
except (NameError, KeyError, UserError):
  crutils.FatalError("The `env` variable is missing", "FATAL INTERNAL ERROR")
  env = None # Unreachable, only for linting

def decodeBootloader(source = None, target = None, env = None):
  if not env:
    crutils.FatalError("The `env` variable is missing", "FATAL INTERNAL ERROR")

  OSCR_CLOCKGEN         = (1 <<  0) # Clock Generator
  OSCR_VSELECT          = (1 <<  1) # VSelect
  OSCR_RTC              = (1 <<  2) # RTC
  OSCR_SSR_LCD          = (1 <<  7) # Using LCD from StarshadeRETRO/BigTreeTech
  OSCR_OBM              = (1 <<  0) # Integrated ATmega2560
  OSCR_CBUS             = (1 <<  1) # Cartridge Bus
  OSCR_VOLTMON          = (1 <<  2) # Voltage Monitors
  OSCR_STAT_LED         = (1 <<  3) # Status LED
  OSCR_SVN_SLT          = (1 <<  5) # Seven-slot Adapter
  OSCR_USBC             = (1 <<  6) # Integrated USB C port
  OSCR_AUX              = (1 <<  7) # AUX Capability
  OSCR_GBX              = (1 <<  0) # GB slot
  OSCR_NES              = (1 <<  1) # NES slot
  OSCR_FC               = (1 <<  2) # Famicom slot
  OSCR_SNES             = (1 <<  3) # SNES/SFC slot
  OSCR_N64              = (1 <<  4) # N64 slot
  OSCR_MD               = (1 <<  5) # Mega Drive/Genesis slot
  OSCR_SMS              = (1 <<  6) # SMS slot
  OSCR_GG               = (1 <<  7) # Game Gear slot

  boardConf = env.BoardConfig()
  bootloaderConf = boardConf.get("bootloader", {})
  port = bootloaderConf.get("port", "")

  if (port == ""):
    crutils.FatalError("No serial port specified")

  click.echo("Detecting bootloader baud rate ... ")

  deviceDetails = crutils.detectbootloaderspeed(port, env)

  if not deviceDetails:
    crutils.FatalError("Unable to communicate with bootloader (Note: The customized OSCR version of urboot is required to use this feature!)")


  click.echo("")
  click.echo(" >>> Got details from OSCR bootloader via {port} @ {baud} baud".format(port=deviceDetails['port'], baud=deviceDetails['baud']))
  click.echo("")
  click.echo("")

  header = "OSCR Hardware Version {ver}, Revision {rev}".format(ver=deviceDetails['ver'], rev=deviceDetails['rev'])

  crutils.Bar()
  crutils.LabeledBar(header, char=" ")

  if (deviceDetails['bldrid'] > 0):
    match (deviceDetails['bldrid']):
      # If you don't care about the number, just take the next available one.
      case 0x01:  builder, buildType = "Rad Rabbit Retro", "Built"
      case 0x69:  builder, buildType = "StarshadeRETRO", "Built"
      case 0x6A:  builder, buildType = "StarshadeRETRO", "Kit"
      case _:     builder, buildType = hex(deviceDetails['bldrid']), "Unknown"

    subHeader = " {type} by {builder} ".format(type=buildType, builder=builder)

    crutils.LabeledBar("{subHeader:*^{headerLen}}".format(subHeader=subHeader, headerLen=len(header)-4), char=" ")

  crutils.Bar()
  sleep(0.5)

  if (deviceDetails['hardware'] > 0):
    click.echo("")
    crutils.LabeledBar("Basic Hardware", char="-")

    if (deviceDetails['hardware'] & OSCR_CLOCKGEN):
      click.echo(" + Clock Generator");
    if (deviceDetails['hardware'] & OSCR_VSELECT):
      click.echo(" + Automatic Voltage Select");
    if (deviceDetails['hardware'] & OSCR_RTC):
      click.echo(" + Real-Time Clock");
    if (deviceDetails['hardware'] & OSCR_SSR_LCD):
      click.echo(" + StarshadeRETRO LCD");

    crutils.Bar(char="-")
    sleep(1)
  else:
    click.echo("!! No basic hardware information is present or none were installed.")

  if (deviceDetails['integrated'] > 0):
    click.echo("")
    crutils.LabeledBar("Integrated Hardware", char="-")

    if (deviceDetails['integrated'] & OSCR_OBM):
      click.echo(" + Integrated ATmega2560");
    if (deviceDetails['integrated'] & OSCR_CBUS):
      click.echo(" + Cartridge Bus");
    if (deviceDetails['integrated'] & OSCR_VOLTMON):
      click.echo(" + Voltage Monitoring");
    if (deviceDetails['integrated'] & OSCR_STAT_LED):
      click.echo(" + Status LED");
    if (deviceDetails['integrated'] & OSCR_SVN_SLT):
      click.echo(" + Seven-Slot Top PCB");
    if (deviceDetails['integrated'] & OSCR_USBC):
      click.echo(" + USB C Port");
    if (deviceDetails['integrated'] & OSCR_AUX):
      click.echo(" + AUX Accessory Port");

    crutils.Bar(char="-")
    sleep(1)
  else:
    click.echo("!! No integrated hardware information is present or none were installed.")

  if (deviceDetails['slots'] > 0):
    click.echo("")
    crutils.LabeledBar("Integrated Slots", char="-")

    if (deviceDetails['slots'] & OSCR_GBX):
      click.echo(" + Game Boy");
    if (deviceDetails['slots'] & OSCR_NES):
      click.echo(" + NES");
    if (deviceDetails['slots'] & OSCR_FC):
      click.echo(" + Famicom");
    if (deviceDetails['slots'] & OSCR_SNES):
      click.echo(" + SNES/SFC");
    if (deviceDetails['slots'] & OSCR_N64):
      click.echo(" + N64");
    if (deviceDetails['slots'] & OSCR_MD):
      click.echo(" + Mega Drive/Genesis");
    if (deviceDetails['slots'] & OSCR_SMS):
      click.echo(" + SMS");
    if (deviceDetails['slots'] & OSCR_GG):
      click.echo(" + Game Gear");

    crutils.Bar(char="-")
    sleep(1)
  else:
    click.echo("!! No integrated slot information is present.")

  click.echo("")
  crutils.LabeledBar("Note", char="-")
  click.echo(" The data above is what your bootloader reports about your OSCR.")
  click.echo(" The values were set by whoever burnt your bootloader to your ATmega2560.")
  click.echo(" Any changes you've made since then will not be reflected above.")
  crutils.Bar(char="-")
  click.echo("")
  crutils.Bar()
  click.echo("")

# Multiple actions
env.AddCustomTarget(
    name="decodebootloader",
    dependencies=None,
    actions=[
        decodeBootloader
    ],
    title="Check OSCR Bootloader",
    description="Decode OSCR Bootloader"
)
