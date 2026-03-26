import shutil
import click # pyright: ignore[reportMissingImports]
from os import listdir, unlink
from os.path import join, dirname, isfile, isdir, islink
from time import sleep
from platform import system, machine
from platformio.util import get_systype # pyright: ignore[reportMissingImports]

def Dir():
    return dirname(__file__)

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

def HostError(message):
    sleep(1)
    click.echo("", err=True)
    LabeledBar(click.style("ERROR", fg="red", bold=True), char="!", err=True)
    click.echo(click.style(message + ":", fg="red", bold=True), err=True)
    click.echo("- Host Operating System: " + system(), err=True)
    click.echo("- Host Architecture: " + machine(), err=True)
    click.echo("- Host System Type (PIO): " + get_systype(), err=True)
    Bar(char="!", err=True)
    click.echo("", err=True)
    exit(1)

def HostWarning(message):
    sleep(1)
    click.echo("", err=True)
    LabeledBar(click.style("WARNING", fg="yellow", bold=True), char="!", err=True)
    click.echo(click.style(message + ":", fg="red", bold=True), err=True)
    click.echo("- Host Operating System: " + system(), err=True)
    click.echo("- Host Architecture: " + machine(), err=True)
    click.echo("- Host System Type (PIO): " + get_systype(), err=True)
    Bar(char="!", err=True)
    click.echo("", err=True)
    return

def ConfigError(message):
    sleep(1)
    click.echo("", err=True)
    LabeledBar(click.style("ERROR", fg="red", bold=True), char="!", err=True)
    click.echo(click.style(message, fg="red", bold=True), err=True)
    Bar(char="!", err=True)
    click.echo("", err=True)
    exit(1)

def GenericError(message):
    sleep(1)
    click.echo("", err=True)
    LabeledBar(click.style("ERROR", fg="red", bold=True), char="!", err=True)
    click.echo(click.style(message, fg="red", bold=True), err=True)
    Bar(char="!", err=True)
    click.echo("", err=True)
    exit(1)

def ToolchainDetails(project_root = None, toolchain_root = None, config_file = None, currentVersion = None, version = None, archive = None):
    click.echo("Toolchain Details:")

    if (project_root != None): click.echo(" ".join(["+ Project Root:", project_root]))
    if (toolchain_root != None): click.echo(" ".join(["+ Toolchain Root:", toolchain_root]))
    if (config_file != None): click.echo(" ".join(["+ Config File:", config_file]))

    if (version != None):
        click.echo(" ".join(["+ Current Version:", (currentVersion if not currentVersion == None else "None")]))
        click.echo(" ".join(["+ Expected Version:", version]))

    if (archive != None):
        click.echo(" ".join(["+ Download URL:", archive['url']]))
        click.echo(" ".join(["+ Archive File Name:", archive['filename']]))
        click.echo(" ".join(["+ Extracted Directory:", archive['directory']]))
    return
