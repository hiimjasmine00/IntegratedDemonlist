# Integrated Demonlist Changelog
## v1.7.7 (2025-05-28)
- Switched to AREDL API v2
- Fixed a bug where the game would crash when trying to load the demonlist if the AREDL API was down

## v1.7.6 (2025-04-04)
- Added support for iOS
- Fixed a bug where the page label would be incorrect when viewing a demon pack

## v1.7.5 (2025-04-01)
- Internal code changes (April Fools' Day)

## v1.7.4 (2025-03-24)
- Fixed a probable bug which could cause demon search to not work properly

## v1.7.3 (2025-02-07)
- Reduced Pemonlist size to 150 demons
- Added node IDs to the list pages

## v1.7.2 (2024-11-15)
- Ported to Geode v4.0.0-beta.1

## v1.7.2-beta.1 (2024-11-13)
- Tweaked the search algorithm to search every part of the demon's name, rather than just the beginning
- Ported to Geometry Dash v2.207

## v1.7.1 (2024-11-01)
- Fixed a bug where the demonlist would not close when the escape key was pressed

## v1.7.0 (2024-11-01)
- Changed ranking text to load individually on cell load rather than all at once in the main menu

## v1.6.5 (2024-10-25)
- Fixed two-player demons not appearing in the demonlist twice (Someone wanted this for some reason)
- Permanently fixed the Pemonlist issue (New API)
- Fixed the star and moon buttons reloading the demonlist

## v1.6.4 (2024-10-24)
- Temporarily fixed a crashing issue with the Pemonlist that would cause the game to not launch

## v1.6.3 (2024-09-04)
- Fixed the Pemonlist not loading at all

## v1.6.2 (2024-09-03)
- Actually fix the issue because my God I can't stand this anymore

## v1.6.1 (2024-09-02)
- Fixed a bug where the enter key would not work at all

## v1.6.0 (2024-09-01)
- Added AREDL pack support
- Fixed a bug where the AREDL would not load at the start of the game

## v1.5.10 (2024-08-31)
- Fixed a bug where the game would crash if the Pemonlist or AREDL button was clicked when the search results were empty

## v1.5.9 (2024-08-30)
- Moved the "Failed to load" notifications to log messages
- Added the status code to the "Load Failed" popup
- Split the AREDL/Pemonlist button into two separate buttons
- Fixed the loading symbol only showing after loading the demonlist

## v1.5.8 (2024-08-22)
- Fixed the source link on the mod info page

## v1.5.7 (2024-08-19)
- Updated the list of AREDL owners
- Added additional links to the mod info page

## v1.5.6 (2024-08-19)
- Fixed a bug where the demonlist page would crash the game if it was exited too quickly

## v1.5.5 (2024-07-28)
- Altered the appearance of the demonlist button

## v1.5.4 (2024-07-27)
- Fixed a bug where the demonlist page would sometimes crash the game

## v1.5.3 (2024-07-20)
- Highlighted the search button when searching for a demon in the demonlist page
- Added a gallery to the mod's about page
- Fixed a bug where clicking the demonlist button would erase the search box of the level search menu

## v1.5.2 (2024-06-21)
- Fixed a bug where the demonlist page would crash the game on macOS

## v1.5.1 (2024-06-20)
- Tweaked the UI of the demonlist page
- Fixed a bug where the refresh button would empty the list
- Fixed a bug where legacy demons would appear in the demonlist and two-player demons would appear twice

## v1.5.0 (2024-06-19)
- Added Pemonlist support
- Fixed a bug where the list would not load if the main menu was exited too quickly

## v1.4.9 (2024-06-14)
- Finalized the release of v1.4.9

## v1.4.9-beta.3 (2024-06-12)
- Fixed a bug where the game would crash when viewing daily and weekly levels

## v1.4.9-beta.2 (2024-06-12)
- Fixed a bug where the search box would stay selected after selecting a demon

## v1.4.9-beta.1 (2024-06-08)
- Fixed a few things

## v1.4.8 (2024-06-07)
- Tweaked the UI of the demonlist page
- Ported to Geometry Dash v2.206

## v1.4.7 (2024-05-15)
- Fixed a bug where two-player demons would appear twice in the demonlist
- Fixed a few bugs with the demonlist search
- Changed the logo to be more in line with Geode's logo style

## v1.4.6 (2024-04-30)
- Tweaked the appearance of the demonlist button
- Fixed incompatibility with DarkMode V4

## v1.4.5 (2024-04-13)
- Fixed a bug where the demonlist button would not appear

## v1.4.4 (2024-03-31)
- Fixed a bug where the back button would not work on Android

## v1.4.3 (2024-03-29)
- Tweaked AREDL preload

## v1.4.2 (2024-03-08)
- Fixed the randomize button on the demonlist page

## v1.4.1 (2024-03-06)
- Added the demonlist to the list search menu

## v1.4.0 (2024-02-25)
- Added Enter as a keyboard shortcut to search for a demon in the demonlist page
- Disabled the page jump buttons when the demonlist is loading or when there is only one page
- Searching with the same query will not return to the first page
- Searching with a different query will now refresh the demonlist

## v1.3.2 (2024-02-25)
- Moved the demonlist button to the level search menu

## v1.3.1 (2024-02-24)
- Added a setting to enable the ranking text on the demon's search box, and enabled it by default
- Added support for Android and macOS

## v1.3.0 (2024-02-21)
- Added keyboard shortcuts to the demonlist page (Esc to close the search menu, Left/Right to navigate between pages)
- Added jump to page functionality to the demonlist page
- Added buttons to jump to the first and last page of the demonlist
- Added a button to jump to a random page of the demonlist
- Fixed a bug where the next page button would be visible even when there are no more pages
- Fixed a bug where the last item index in the page label would be incorrect when the last page is not full

## v1.2.0 (2024-02-17)
- Moved the demonlist button to the list search menu

## v1.1.0 (2024-02-16)
- Added a refresh button to the demonlist page
- Added an alert when the search fails
- Added corners to the demonlist page
- Fixed a bug where the game would crash when leaving the demonlist page while loading
- Fixed a bug where the loading symbol would not reappear after the first time

## v1.0.0 (2024-02-14)
- Initial release