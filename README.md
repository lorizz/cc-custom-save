
# Custom Save 2.0 - Castle Crashers ASI Mod

**Custom Save 2.0** is an ASI mod for _Castle Crashers_ that enhances character management by allowing the use of external save files. This enables players to have more than the default 31 characters and to customize character attributes easily through `.json` files.

## Features

-   **Expanded Character Slots**: Manage virtually unlimited characters beyond the default 31.
    
-   **Customizable Attributes**: Add or modify character attributes beyond the standard ones (e.g., adding a "critRate" attribute).
    
-   **External Save Management**: Character data is stored in easily editable `.json` files.
    
-   **Organized Save Structure**: Default characters are stored in the `saves/characters` folder, while additional characters are in `saves/characters/addon`.
    

## Important Disclaimer

**⚠️ WARNING: Before proceeding, it is **crucial** to back up your original save files.**

The original save files are located at:

```
<Steam-folder>/userdata/<user-id>/204360/Remote

```

Additionally, a solid understanding of SWF files and basic programming knowledge are **highly recommended** to effectively configure and utilize this mod.

## Installation Guide

1.  **Backup Original Save Files**: As emphasized above, ensure you have a backup of your original save files.
    
2.  **Download and Extract the Mod**: Obtain the Custom Save 2.0 mod files and extract them into your _Castle Crashers_ game directory.
    
3.  **Configure `customsave.ini`**: This file defines the data structure and additional characters.
    
    -   **Character Data Structure**: Specify the sequence and byte size of each attribute. For example:
        
        ```ini
        ; Where follows the structure: fieldName, byteSize
        [CharacterDataStructure]
        unlockStatus,1
        level,1
        xp,4
        weapon,1
        pet,1
        stats,4
        normalLevelUnlocks,3
        consumableItems,3
        nonConsumableItems,1
        money,4
        insaneModeStoreUnlocks,1
        insaneLevelUnlocks,3
        skullStatus,1
        dugUpItems,1
        princessKisses,1
        arenaWins,2
        unknown,16
        
        ```
        
        If you add a new attribute (e.g., `critRate`), append it in both the `[CharacterDataStructure]` section and the corresponding SWF file, maintaining the same order.
        
    -   **Addon Characters**: List additional characters to be added:
        
        ```ini
        ; Where each params are separated by the comma (,) and are in this order: id, startingWeaponId, isInitiallyUnlocked
        [AddonCharacters]
        testGuy1,1,true
        testGuy2,1,true
        ```
        
        Ensure that the order here matches the order in the modified SWF files to prevent data mismatches.
        
4.  **Modify `main.swf`**: Adjust the `f_InitSaveSystem()` function to reflect the new data structure and character count:
    
    ```actionscript
    function f_InitSaveSystem()
    {
       save_data_info = new Object();
       save_data_info.char_offset = 64;
       save_data_info.char_size = <NEW_BYTE_SIZE>; // Update to match the total byte size of the new data structure
       save_data_info.num_items = 128;
       save_data_info.num_animals = 32;
       save_data_info.num_levels = 64;
       save_data_info.num_relics = 8;
       save_data_info.num_items_expansion = 64;
       save_data_info.num_characters = <TOTAL_CHARACTER_COUNT>; // 31 + number of addon characters
       relic_offset = 40;
       weapon_offset = 50;
    }
    
    ```
    
    -   **Update `char_size`**: Set this to the total byte size of your updated character data structure.
        
    -   **Update `num_characters`**: Set this to `31 +` the number of addon characters you've added.
        
5.  **Adjust Character Data Handling**: In the `f_LoadCharacterData()` and `f_FlushSaveData()` functions, ensure that `ReadStorage` and `WriteStorage` calls align with the sequence defined in `customsave.ini`. For example, if you've added a `critRate` attribute under the `stats` property in the `customsave.ini`:
    
    ```actionscript
    WriteStorage(_loc5_, _loc4_.agility);
    WriteStorage(_loc5_, _loc4_.critRate); // Ensure this follows the same order as in customsave.ini
    
    ```
    

## Configuration Notes

-   **Attribute Consistency**: When adding new attributes, ensure they are consistently defined in `customsave.ini`, the SWF files, and the game's data handling functions.
    
-   **Order Matching**: The order of characters in `customsave.ini` must match the order in the SWF files to prevent data from being assigned incorrectly.
    
-   **Data Integrity**: Modifying game files carries risks. Always double-check configurations and maintain backups to prevent data loss or corruption.
    

----------

By following this guide, you can expand and customize your _Castle Crashers_ experience with new characters and attributes. Remember to proceed with caution and ensure all configurations are consistent to maintain game stability.
