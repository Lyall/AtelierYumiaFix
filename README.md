# Atelier Yumia Fix
[![Patreon-Button](https://github.com/Lyall/AtelierYumiaFix/blob/main/.github/Patreon-Button.png?raw=true)](https://www.patreon.com/Wintermance) 
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)<br />
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/AtelierYumiaFix/total.svg)](https://github.com/Lyall/AtelierYumiaFix/releases)

**AtelierYumiaFix** is an ASI plugin for Atelier Yumia: The Alchemist of Memories & the Envisioned Land that can:
- Add support for custom resolutions.
- Fix HUD issues at ultrawide/narrower resolutions.

## Installation  
- Download the latest release from [here](https://github.com/Lyall/AtelierYumiaFix/releases). 
- Extract the contents of the release zip in to the the game folder.  

### Steam Deck/Linux Additional Instructions
🚩**You do not need to do this if you are using Windows!**  
- Open the game properties in Steam and add `WINEDLLOVERRIDES="dinput8=n,b" %command%` to the launch options.  

## Configuration
- Open **`AtelierYumiaFix.ini`** to adjust settings.

## Known Issues
**Ultrawide/Narrower**
- When in battle, the small visual effect that appears upon activating a skill is displayed at the wrong aspect ratio.
- At wider than 21:9 aspect ratios, the crouch vignette does not span the screen fully.
- At wider than 32:9 aspect ratios, the minimap image is duplicated at the edge of the screen.

## Screenshots
| ![ezgif-19f656bb6c209b](https://github.com/user-attachments/assets/bafa5269-5a96-4b48-9e6e-a9754973b96d) |
|:--:|
| Gameplay |

## Credits
Special thanks to RenaSera for commissioning this fix! <br/>
[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading. <br />
[inipp](https://github.com/mcmtroffaes/inipp) for ini reading. <br />
[spdlog](https://github.com/gabime/spdlog) for logging. <br />
[safetyhook](https://github.com/cursey/safetyhook) for hooking.
