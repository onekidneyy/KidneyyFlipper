# Opensource Applications For The Flipper Zero

These will need to be manually added one by one in the applications_user folder in your firmware and then launched on your flipper to make it a fap file.

## Overview Of The Open Source Apps

* Sample App

Sample folder holds a base setup to create apps for the Flipper Zero that has three menus a Config, Play, and About.

* Skeleton App

Skeleton folder holds a more advanced version of the Sample App this setup has all the menus interconnected for you to see how it could look.

* Solana Wallet App (WIP)

* To DO List App (WIP)

## Downloading Firmware

To download your firmware go to its respected github repo and copy the HTTPS and do a recursive clone of it in your code editior. Then build the firmware by pressing crtl,shift,b and picking the "(Debug) Build Firware" on momentum and "(Debug) Build" on flippers firmware.

## Downloading Toolchain For VScode

To download the toolchain for VS code right click the fbt file and then open it in a integrated terminal and in the terminal type ./fbt vscode_dist

## Updating application_user Folder

Decide what folder from this application_user folder you want to add into your firmwares application_user folder

## Launching App/Making it a FAP File

Once you want to launch the app on your flipper press crtl,shift,b and pick "(Debug) Launch App on FlipperZero" and that will launch the app on your flipper and make it a FAP file and put it in its correct app location.
