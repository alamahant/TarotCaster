# Changelog

## [1.1] - 2025-04-29

### New Content
- Added the classic Tarot of Marseilles deck, sourced from Wikimedia Commons [View on Wikimedia](https://commons.wikimedia.org/wiki/Category:Tarot_de_Marseille_(Single_Cards))
- Introduced the Zodiac spread, offering astrological insights with a 12-card layout representing each zodiac sign

### User Interface Improvements
- Redesigned application icon with an elegant blue background and golden yellow text
- Consolidated all styling into a central CSS file for consistent appearance throughout the application
- Added more screenshots to better showcase the application's features

### New Features
- Enhanced Help menu with comprehensive sections:
  - About: Application information and credits
  - Instructions: Detailed guide for beginners and experienced users
  - Spreads: Explanations of available card layouts and their meanings
  - Custom Decks: Step-by-step instructions for adding personal decks
- Implemented user-friendly deck management system:
  - Created standardized user "decks" directory that follows Flatpak sandbox guidelines
  - Automatic detection of custom decks placed in `~/.var/app/io.github.alamahant.TarotCaster/data/TarotCaster/decks/`
  - No restart required for the application to recognize newly added decks

### Technical Improvements
- Improved random number generation for more authentic shuffling:
  - Enhanced seed generation at application startup
  - Additional entropy collection during the shuffle process
- Optimized save/load functionality for better compatibility with Flatpak sandbox environment
- Updated application summary to conform with Flathub quality guidelines

### Bug Fixes
- Resolved issue where saved readings would display cards face-down upon loading
- Fixed various minor UI inconsistencies and layout issues

## [1.0] - 2025-04-09

- Initial release