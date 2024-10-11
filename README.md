# Custom Save - Castle Crashers ASI Mod

**Custom Save** is an ASI mod for Castle Crashers that allows modders to add up to **1000 characters** into the game's memory. This mod operates independently of the Steam save file and comes with a patched `castle.exe` to remove Steam's DRM, enabling the injection of multiple ASI mods. The save file is generated upon first execution in the `scripts/saves/` folder and can be freely edited by the modder.

## Features
- Add up to 1000 characters in the game's memory.
- Custom save is stored in `scripts/saves/game_data.json`.
- Fully independent from the Steam save file.
- Includes a patched `castle.exe` that removes Steam DRM (necessary for ASI modding).
- Unknown bytes in the save file allow further exploration and customization by the modder.

## Installation

1. **Backup** the original `castle.exe` by copying it to a safe location.
2. Download and unzip the mod files into the game's main folder.
3. On the first run, the mod will generate a save file at `scripts/saves/game_data.json` containing the original 31 characters with default values.

## How It Works

Custom Save modifies the `WriteStorage` and `ReadStorage` functions of the game executable to read from a custom save file instead of the Steam save file.

### Default Save File

When first initialized, the file `game_data.json` is generated with the original 31 characters. If you want to add more characters:

1. Add the new characters below the existing ones in the `game_data.json` file.
2. The order of characters is **important** since the game loads them sequentially. **Do not** change the order of the original characters as this can break the graphics and unlock logic.

### Setting Total Characters

After adding your custom characters, it is **crucial** to update the `main.swf` file to reflect the total number of characters. 

If you have added new characters, you need to modify the value of `save_data_info.num_characters` in the `main.swf` file to match the new total. For example, if you now have 40 characters, ensure `num_characters` is set to 40.

> **IMPORTANT:** Setting `save_data_info.num_characters` correctly is essential to avoid game-breaking issues.

## JSON Data Structure

- All values in the JSON are written in **decimal** format. The mod will automatically convert them to **hexadecimal** for the game.
- The unlock status is represented as:
  - `0` = **locked**
  - `128` = **unlocked** (`128` = `0x80` in hexadecimal)

## Notes

Some bytes in the save file are still unknown and may offer additional modding opportunities. Further exploration is encouraged for advanced modders.

---

Enjoy expanding your Castle Crashers roster with **Custom Save**! For any issues or inquiries, feel free to open an issue or contribute to the project.
