# Custom Save (Castle Crashers Mod)

**Custom Save** is an **ASI mod** for *Castle Crashers* that replaces the vanilla `cc_save.dat` binary system with a fully structured **JSON-based save system**.
It is built on top of the [HookCrashers API](https://github.com/lorizz/hookcrashers).

---

## ✨ Features

* Save files are now structured in **JSON** format instead of raw binary.
* Characters are organized into folders:

  * **base/** → Default Castle Crashers characters (up to *Painter JR*).
  * **workshop/** → Workshop characters (IDs 1–10).
  * **addon/** → Custom user-defined characters.
* **No limit** on the number of base characters: you can add new ones after *Painter JR*.
* A new file `globalUnlocks.json` represents the first 64 bytes of the original save and can be freely edited.
* Introduces a new function for SWF files:
  **GetCustomSaveData(string param)** → Dynamically returns save parameters such as number of characters, items, animals, etc.

⚠️ Currently supported only on the **New Graphic version** of Castle Crashers, and requires the **Painter Boss DLC**.

---

## 📥 Installation

1. Download the mod files.
2. Place `CustomSave.asi` into your **Castle Crashers/mods/** folder.
3. Modify your **SWF files** (e.g. `main.swf`) by replacing the `f_InitSaveSystem()` function as explained below.

---

## 📂 Save Structure

After installation, the new save data will be located in:

* **globalUnlocks.json** → Contains the first 64 bytes of the original save.
* **base/** → Characters from `greenKnight` to `painterJr`.
* **workshop/** → Characters 1–10.
* **addon/** → User-created characters.

Folder layout:

```
mods/CustomSave/
├── globalUnlocks.json
├── base/
│   └── [base characters].json
├── workshop/
│   └── [workshop characters].json
└── addon/
    └── [custom characters].json
```

---

## ➕ Adding a New Character

1. Open `CustomSave.ini`.
2. Add your custom character definition under `[AddonCharacters]`.

Example:

```
; Structure => id_name,id_weapon,id_pet,initially_unlocked
; Example:
; customChar,32,0,true
; lockedChar,51,2,false

[AddonCharacters]
myCustomChar,45,1,true
```

3. Modify your `.swf` files (`lobby.swf`, `main.swf`, etc.) to insert the new character frame after *Painter JR*.

---

## 🛠️ Required SWF Modification

To make the save system work dynamically, you must replace the original `f_InitSaveSystem()` in your SWFs with this version:

```actionscript
function f_InitSaveSystem()
{
   save_data_info = new Object();
   save_data_info.char_offset = GetCustomSaveData("char_offset");
   save_data_info.char_size = GetCustomSaveData("char_size");
   save_data_info.num_items = GetCustomSaveData("num_items");
   save_data_info.num_animals = GetCustomSaveData("num_animals");
   save_data_info.num_levels = GetCustomSaveData("num_levels");
   save_data_info.num_relics = GetCustomSaveData("num_relics");
   save_data_info.num_items_expansion = GetCustomSaveData("num_items_expansion");
   save_data_info.num_characters_legacy = GetCustomSaveData("num_characters_legacy");
   save_data_info.num_characters_noaddons = GetCustomSaveData("num_characters_noaddons");
   save_data_info.num_characters_safe = GetCustomSaveData("num_characters_safe");
   save_data_info.num_character_addons = GetCustomSaveData("num_characters_addons");
   save_data_info.num_characters = GetCustomSaveData("num_characters");
   relic_offset = 40;
   weapon_offset = 50;
}
```

This ensures that the game dynamically handles characters, items, animals, and other save data without needing to hardcode new values every time.

---

## 🔮 Future Plans

* Support for multiple save slots (switch between a clean run and your main save).
* Online database for sharing and downloading saves.
* Automatic character loading via files (no more manual SWF edits).

---

## 📌 Requirements

* [HookCrashers API](https://github.com/lorizz/hookcrashers)
* Castle Crashers (*New Graphic version*)
* Painter Boss DLC