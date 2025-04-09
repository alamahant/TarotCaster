## Troubleshooting

### XFCE Panel Launcher Issue

Some XFCE users may experience difficulty adding TarotCaster to the panel using the standard launcher. This is due to a known compatibility issue between XFCE's panel launcher and certain Flatpak applications.

#### Temporary Solution

If you're using XFCE and cannot add TarotCaster to your panel, follow these steps:

1. Create a custom desktop entry file:

```bash
mkdir -p ~/.local/share/applications/
cat > ~/.local/share/applications/tarotcaster-fixed.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=TarotCaster
GenericName=Tarot Card Reader
Comment=AI-powered tarot card reading application
Icon=io.github.alamahant.TarotCaster
Exec=/usr/bin/flatpak run --branch=stable --arch=x86_64 --command=TarotCaster io.github.alamahant.TarotCaster
Terminal=false
Categories=Utility;Game;
Keywords=tarot;cards;reading;divination;ai;
StartupNotify=true
X-Flatpak=io.github.alamahant.TarotCaster
EOF
```

2. Update the desktop database:

```bash
update-desktop-database ~/.local/share/applications/
```

3. Now you can add TarotCaster to your XFCE panel:
   - Right-click on the panel
   - Select "Add New Items"
   - Choose "Launcher"
   - Find and select TarotCaster from the applications list

This issue will be fixed in the next update of TarotCaster.

#### Alternative Method

You can also create a custom launcher directly in the XFCE panel:
1. Right-click on the panel
2. Select "Add New Items"
3. Choose "Launcher"
4. Click "Create a new empty launcher"
5. Fill in the following details:
   Name: TarotCaster
   Let the launcher fill in the remaining fields.

#### Alternative Method
Please use XFCE Applictaion Finder to locate and launch TarotCaster or use the command line invocation:
flatpak run io.github.alamahant.TarotCaster

This issue will be fixed in the next update of TarotCaster.

  