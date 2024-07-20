# Skeleton Folder

You can use this application as a starting point for creating your own application.  It contains the skeleton framework to get an application running on the Flipper Zero.

## Overview

This application has three submenu items:

* Config
* Play
* About

## Config

The "Config" menu item currently has 2 settings.  The first setting has a selection of 3 options (and demonstrates how the numeric values associated with them don't have to be sequential.)  The other setting has a text field which is set by clicking OK while selecting the setting (depending on your application, you may want to do something else when an item is clicked, like bring up a hex selector.)

## Play

The "Play" screen is where you would put your primary application.  The current implementation renders some data.  When Left/Right buttons are clicked the value of x changes and the icon moves left/right.  Up/Down buttons don't do anything in our implementation.  As soon as the OK button is pressed, a tone is made based on the value of x.  Pressing the back button goes back to the menu.

## About

The "About" menu item contains information about your application, so people know what to do with it & how to contact you.
