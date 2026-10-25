# PBL-SDF1
Mini Desktop OS Simulation in C (raylib)
A Windows Vista-inspired desktop environment built in C with raylib as our 1st-semester PBL project. It simulates a desktop with a wallpaper, a Start orb, a taskbar with a live clock and date, and four working apps, each written in its own C file and loaded through a shared apps.h header.

Features

Desktop icons for Calculator, Notepad, Image Editor and Music Player. Click one to open the app in its own window, and close it with the X button or Esc.
Right-click an icon for an Open / Rename / Delete menu. Renaming uses a text input box.
Right-click the desktop and choose Refresh to restore deleted or renamed icons.
Graceful fallbacks: if the wallpaper, icons or font are missing, it draws a gradient wallpaper, letter icons and the default font instead of crashing.
Modular design: every app lives in its own file, so team members could work independently.

Built with: C, raylib
