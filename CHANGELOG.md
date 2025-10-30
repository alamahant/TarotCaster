# Changelog

## [1.2.2] - 2025-10-29

### Technical Updates
- **Updated KDE Runtime**:
  - Upgraded KDE Flatpak runtime to version 6.9 for improved performance and compatibility
  - Ensures better system integration and future-proof security

- **New Feature: Question Box**:
  - Added a question box, so that the user can set a question before dealing the cards
  - Then the ai will interpret the spread taking in cosideration the user's question

- **Enhanced AI Response Capacity**:
  - Updated MistralAPI class to increase allocated token limit for AI responses
  - Allows for more detailed and comprehensive spread interpretations
  - Better handles longer, in-depth readings without truncation

### Visual Enhancements
- **HTML Rendering for AI Responses**:
  - Implemented markdown-to-HTML conversion for AI spread interpretations
  - Delivers visually stunning rendition of tarot readings with proper formatting
  - Enhances readability and presentation of AI-generated content

### Technical Improvements
- Performed comprehensive code polishing and optimization
- Various under-the-hood improvements for stability and maintainability
- Refined existing features for smoother user experience

## [1.2.1] - 2025-07-31

### New Features
- **Sola Busca Tarot Deck Support**:
  - Added full support for the historical Sola Busca Tarot deck
  - Images sourced from Wikimedia Commons and rendered with high fidelity
  - Deck selectable from the deck manager alongside Rider-Waite and Marseille

- **Universal Zoom Slider**:
  - Added a global zoom control slider to set overall card scaling
  - Affects all cards in the spread uniformly for personalized viewing comfort

- **Strength–Justice Swap Option**:
  - Added checkbox setting to swap cards VIII and XI (Strength and Justice)
  - Supports historical deck orders like Tarot de Marseille
  - Swap applies dynamically per selected deck where applicable

- **Dock Visibility Toggles**:
  - Under the **View** menu, users can now toggle visibility of left/right docks
  - Allows full focus on the card spread without UI distractions
 
- **Deck Renaming Utility Dialog**:
  - New tool to help users rename cards of custom digital decks
  - Automatically converts filenames to the naming convention used by TarotCaster
  - Ensures smooth import and compatibility with app features

### Visual and Technical Enhancements
- Improved card rendering clarity with updated scaling and anti-aliasing techniques
- Enhanced sharpness and detail for all decks, especially at higher zoom levels
- Continued optimization of rendering pipeline for smoother performance
- General code cleanup and minor polish for better maintainability and stability

## [1.2.0] - 2025-05-16

### New Features
- **Custom Spread Designer**:
  - Users can now create and save their own custom card layouts
  - Intuitive drag-and-drop interface for positioning cards
  - Option to name and describe the spiritual significance of each position
  - Save and load custom spreads for future readings
  - Share custom spreads with other users

- **Card-specific Zoom Functionality**:
  - Users can now Ctrl+mouse wheel while hovering over a card to examine details
  - Zoom starts at the hover magnification (1.5x) and can increase up to 3x
  - Cards automatically return to normal size when no longer hovered
  - Zoom affects only the specific card being examined, not the entire spread

### Visual Enhancements
- Optimized card rendering with pre-scaling in CardLoader for improved performance and image quality
- Maintained high-quality card hover effect with golden shadow highlighting
- Ensured smooth transitions between normal and highlighted card states
- Implemented OpenGL rendering for hardware-accelerated graphics performance

### Technical Improvements
- Implemented proper event handling for wheel events in card items
- Ensured compatibility between hover effects and zoom functionality
- Maintained visual consistency of shadow effects during zoom operations
- Optimized rendering with Qt::SmoothTransformation for high-quality card display
- Enabled OpenGL acceleration for smoother animations and effects
- Utilized GPU-accelerated compositing for shadow effects and transparency
- Added fallback rendering path for systems without OpenGL support
- Implemented pre-scaling system that scales all card images once during loading rather than repeatedly during display
- Added storage for pre-scaled images to eliminate redundant scaling operations

### User Experience
- Enhanced card examination capabilities while preserving the context of spreads
- Improved accessibility by allowing users to see card details more clearly
- Maintained intuitive interaction model with consistent visual feedback
- Significantly improved animation smoothness and overall responsiveness
- Reduced CPU usage during card animations and transitions

### System Requirements
- Added support for hardware acceleration on compatible systems
- Maintained compatibility with systems lacking dedicated graphics hardware

## [1.1.0] - 2025-04-15

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

## [1.0.0] - 2025-04-09
- Initial release

