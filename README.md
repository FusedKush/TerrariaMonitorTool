# TerrariaMonitorTool
A simple command-line tool for Windows used to change the display monitor that [*Terraria*](https://terraria.org/) renders the game on.


## Table of Contents
- [What?](#what)
- [How?](#how)
- [Why?](#why)
- [Usage](#usage)
- [Problems?](#problems)
- [Disclaimer](#disclaimer)


## What?
The *Terraria Monitor Tool* is a command-line program written in C++ and utilizing the Windows API to edit the display monitor that *Terraria* will render the game on by modifying the configuration file that *Terraria* uses to store user settings.


### Features
- Lightweight and Portable
- Stateless Mode and Support for Clearing Program Data
- Support for Custom Game Directories
- Support for Multiple Configuration Files
- Automatic Configuration File Backups *(Coming Soon!)*
- Default Configuration File / Display Monitor Preferences *(Coming Soon!)*
- Support for Custom Display Resolutions *(Coming Soon!)*
- Non-Interactive Mode *(Coming Soon!)*


## How?
*Terraria* stores the monitor it renders the game on within the `Display` and `DisplayScreen` properties of the `config.json` file where most user settings and preferences are stored. E.g.,

```json
{
    "Display": "\\\\.\\Display1",
    "DisplayScreen": "\\\\.\\Display1",
}
```

By changing the value of these properties in the `config.json` file, *Terraria* will launch the game on the correct monitor instead of the monitor it was previously being launched on.

The `config.json` file is located in the *Terraria* Game Data Directory in the User `Documents` folder, which can generally be found in one of two default locations:
```
C:\Users\YOUR_USERNAME\Documents\My Games\Terraria
```
or
```
C:\Users\YOUR_USERNAME\OneDrive\Documents\My Games\Terraria
```


## Why?
From time to time, when playing *Terraria* on my multi-monitor setup, the game would launch on the wrong monitor with no in-game solution available. It turns out that *Terraria* records the monitor to render the game on in the `config.json` file using its generic display identifier, which looks something like `\\.\Display1`. Because *Windows* is free to assign these identifiers however it chooses, the monitors associated with each identifier sometimes change, causing *Terraria* to render the game on a different monitor.

> For example, suppose we have two monitors:
> - *Monitor A*, which has a generic display identifier of `\\.\Display1`
> - *Monitor B*, which has a generic display identifier of `\\.\Display2`
> 
> Now, suppose that when you launch *Terraria* for the first time, it uses the Main Monitor, which happens to be *Monitor A*. Terraria will save `\\.\Display1` to the `config.json` file and launch on *Monitor A*.
>
> However, at any point in time, Windows may decide to reassign the connected monitors in the system to different idenfiers, at which point:
> - *Monitor A*, now has a generic display identifier of `\\.\Display2`
> - *Monitor B*, now has a generic display identifier of `\\.\Display1`
>
> Maybe you can see where this is going? Because *Terraria* only knows that the monitor it should render the game on by the identifier `\\.\Display1`, the game will be launched on *Monitor B*, rather than on *Monitor A*.

As a result, there are generally two solutions to getting *Terraria* to render the game on the correct monitor:
   1. Modify the `config.json` file to reflect the new generic display identifier of the desired monitor.
   2. Sign out or restart your computer, or physically reseat your display cables and hope that *Windows* decides give the monitors the correct identifiers.


## Usage
```
TerrariaMonitorTool [ /?|--help|--usage [<Option or Switch>] ] [ -v | --version ]
                    [ -d|--dry-run ] [ -s|--stateless ] [ -y|--yes ]
                    [ -b|--disable-custom-buffer-behavior ]
                    [ --clear-program-data ] [ --debug ]
```

The program can be launched directly from the program executable or via the command line. Command-Line Flags can be added when launched via the command line or by creating a Windows Shortcut and adding them to the end of the `target`.

| Flag(s)                                   | Description                                                                               |
| ----------------------------------------- | ----------------------------------------------------------------------------------------- |
| `/?`. `--help`, `--usage`                 | Get help and usage information                                                            |
| `-v`, `--version`                         | [Display Version Information](#version-details)                                           |
| `-d`, `--dry-run`                         | [Don't write changes to the Configuration File](#dry-run-mode)                            |
| `-s`, `--stateless`                       | [Skips reading from or writing to any program files](#stateless-mode)                     |
| `-y`, `--yes`                             | [Automatically answer "yes" to all Confirmation Prompts](#automatically-confirm-prompts)  |
| `-b`, `--disable-custom-buffer-behavior`  | [Disable custom behavior for Console Output Buffers](#disable-custom-buffer-behavior)     |
| `--clear-program-data`                    | [Clear existing Program Data before launch](#clear-program-data-before-launch)            |
| `--debug`                                 | [Enable functionality useful for debugging](#debug-friendly-mode)                         |


### Version Details
```
TerrariaMonitorTool [ -v | --version ]
```

Displays Version Information about the Program.


### Dry Run Mode
```
TerrariaMonitorTool [ -d | --dry-run ]
```

Writes the contents of the modified Terraria Configuration File to the Console instead of saving them to the Configuration File.


### Stateless Mode
```
TerrariaMonitorTool [ -s | --stateless ]
```

Skips any optional reading from or writing to any program files.

As a result, no changes to any existing program data will be made by the program, even if changes are made to the Program Settings.

This option does *not* effect whether or not changes are made to the Terraria Configuration File or any Backup Files (see [`--dry-run`](#dry-run-mode) instead).


### Automatically Confirm Prompts
```
TerrariaMonitorTool [ -y | --yes ]
```

Automatically confirms or answers "yes" to any Confirmation Prompts.

This flag should be used with caution, as it permits for potentially dangerous actions to proceed without any further confirmation.

This flag applies to Confirmation Prompts triggered in response to both Comamnd-Line Arguments, as well as Interactive Menu Selections.


### Disable Custom Buffer Behavior
```
TerrariaMonitorTool [ -b | --disable-custom-buffer-behavior ]
```

Disables custom behavior for the Console Output Buffers, including tweaking how the Console is Cleared and the way Alternate Output Buffers Work.

If you are encountering issues with how the program is rendered when switching between screens or clearing the console, you can try running the program with this flag.


### Clear Program Data Before Launch
```
TerrariaMonitorTool [ --clear-program-data [ -y | --yes ] ]
```

Clear any existing Program Data before launching the program.

Applies to files used internally by the program, as well as any existing Backup Configuration Files. This does *not* apply to any Terraria Configuration Files that are not being used as backups.

This action requires confirmation before proceeding. To proceed automatically when launching the program, you can use the [`--yes`](#automatically-confirm-prompts) flag.


### Debug-Friendly Mode
```
TerrariaMonitorTool [ --debug ]
```

Enables functionality useful for debugging, including enabling additional logging
and adding a delay at the start of the program for debuggers to be attached.


## Problems?
- [Issue Tracker](https://github.com/FusedKush/TerrariaMonitorTool/issues)
- [Known Issues](https://github.com/FusedKush/TerrariaMonitorTool/labels/known-issue)


## Disclaimer
> While every effort has been reasonably made to ensure this program never leaves any Terraria Configuration Files or other user data in a corrupt or invalid state, the author of the program is *not* responsible or liable for any destroyed, lost, or malformed data as a result of using this program, especially if you choose to use the program in a way for which it was not explicitly intended.