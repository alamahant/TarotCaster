# TarotCaster

An AI-powered tarot reading application built with Qt that combines traditional tarot wisdom with modern artificial intelligence.

![TarotCaster Screenshot](screenshots/Screenshot8.png)

## Features

- **Multiple Tarot Decks Support**: Includes the classic Rider-Waite deck with support for adding your own custom decks
- **Various Spread Types**: Choose from multiple layouts including:
  - 1-card draw
  - 3-card spread
  - Celtic Cross spread
  - Horseshoe spread
  - More spreads coming soon!
- **AI-Generated Readings**: Powered by Mistral AI for insightful and personalized card interpretations
- **Traditional Card Meanings**: Built-in traditional interpretations for all cards
- **Show Full Deck Feature**: Browse and explore the entire deck of cards
- **Enhanced Randomization**: Shuffle button gathers system entropy for truly random card selection
- **Save/Load Functionality**: Save your readings to revisit them later
- **Intuitive Interface**: Clean, user-friendly design suitable for both beginners and experienced readers

## Installation

### Flatpak (Coming Soon)
```bash
flatpak install io.github.alamahant.TarotCaster
```

### From Source
```bash
git clone https://github.com/alamahant/TarotCaster.git
cd TarotCaster
mkdir build && cd build
cmake ..
make
```

## Using AI Interpretations

TarotCaster uses Mistral AI to generate personalized card interpretations. To use this feature:

1. Register for a free Mistral AI API key at [https://mistral.ai/](https://mistral.ai/)
2. Enter your API key in the application settings
3. Select "Get Reading" when viewing spread

The free tier of Mistral AI provides sufficient requests for personal tarot reading use.

## Adding Custom Decks

You can add your own tarot decks to TarotCaster:

1. Create a folder with your deck name in the `decks` directory
2. Name your card images according to the following convention:
-->Detailed instructions for custom deck creation will be provided in the wiki.
3. Restart the application to see your new deck

## Dependencies

### For Flatpak Users
No additional dependencies required - everything is included in the Flatpak package.

### For Users Building from Source
- Qt 6.2 or higher
- CMake 3.16 or higher
- Standard build tools (gcc/g++, make, ninja, etc)

#### On Ubuntu/Debian-based systems:
```bash
# Install build tools and dependencies
sudo apt install build-essential cmake ninja-build pkg-config

# Install Qt6 (Ubuntu 22.04+)
sudo apt install qt6-base-dev
```

For older Ubuntu versions that don't have Qt6 in their repositories:
```bash
# Add Qt repository
sudo add-apt-repository ppa:okirby/qt6-backports
sudo apt update
sudo apt install qt6-base-dev
```

#### On Fedora/RHEL-based systems:
```bash
# Install build tools
sudo dnf install cmake gcc-c++ ninja-build pkg-config

# Install Qt6
sudo dnf install qt6-qtbase-devel
```

#### On Arch Linux and derivatives:
```bash
# Install build tools
sudo pacman -S base-devel cmake ninja pkg-config

# Install Qt6
sudo pacman -S qt6-base
```

### For Users Running the Binary Directly
The application requires Qt 6.2 libraries or higher to be installed on your system. Most modern Linux distributions will have the other required dependencies already installed.

## Using AI-Powered Readings

TarotCaster integrates with Mistral AI to provide personalized interpretations of your tarot spreads. Follow these steps to enable and use this feature:

### Setting Up Your Mistral API Key

1. **Obtain a Mistral API Key**:
   - Create an account at [Mistral AI](https://mistral.ai/)
   - Navigate to your account dashboard
   - Generate a new API key

2. **Add Your API Key to TarotCaster**:
   - Open TarotCaster
   - Go to the **Edit** menu
   - Select **Mistral API Key**
   - Paste your API key in the dialog box
   - Click "OK" to save

### Getting AI Interpretations

Once your API key is set up:

1. Cast the Tarot cards using any of the available spreads
2. After the cards are displayed, click the **Get Reading** button
3. TarotCaster will connect to Mistral AI and generate a personalized interpretation
4. The AI reading will appear, offering insights based on the specific cards in your spread

### Tips for AI Readings

- Readings are generated based on traditional card meanings and their relationships
- Each reading is unique and tailored to your specific card combination

Note: Using the AI feature requires an internet connection and consumes Mistral API credits. The free tier of Mistral AI typically provides sufficient credits for personal use.

## The Shuffle Feature: Creating a Personal Connection

TarotCaster offers a unique way to "shuffle" the cards that creates a personal energetic connection between you and your reading through digital entropy generation.

### How to Use the Shuffle Feature

1. **Activate Shuffle Mode**:
   - Click the **Shuffle** button in the interface
   - Your cursor will change to a crosshair (+) symbol
   - This indicates that entropy collection is active

2. **Generate Entropy**:
   - Move your mouse cursor around the screen in any pattern
   - Your mouse movements create unique entropy that feeds into TarotCaster's quantum random number generator
   - This process creates a personalized randomization for your reading

3. **Recommended Shuffling Technique**:
   - For a deeper connection, click the **Display Full Deck** button
   - Move your cursor over the displayed cards, following your intuition
   - Take your time with this process - the longer you move, the more unique your shuffle
   - When you feel ready, click the **Display Full Deck** button again to return to the main interface

4. **Complete the Process**:
   - After shuffling, click the **Deal** button to lay out your spread
   - The cards will be selected based on the entropy you generated during shuffling

### Why This Matters

This shuffling method creates a more meaningful connection between you and your reading by:

- Incorporating your physical movements into the card selection process
- Creating truly random card selections based on quantum principles
- Allowing your intuition to guide the shuffling process
- Establishing a ritual similar to physical card shuffling

Even if you don't display the full deck, simply moving your cursor around in shuffle mode will generate the entropy needed for a personalized reading.

## Adding Your Own Tarot Decks

TarotCaster allows you to add your own custom tarot decks. Follow these instructions to prepare and install your deck images.

### Preparing Your Deck Files

1. **Obtain digital images** of your preferred tarot deck
   - Ensure you have rights to use these images
   - PNG, JPG, and JPEG formats are supported
   - Consistent image dimensions are recommended

2. **Rename the files** according to TarotCaster's required format:
   
   **Major Arcana (22 cards):**
   - Name files from `00.png` to `21.png` (or .jpg/.jpeg)
   - Follow the standard order: Fool (00), Magician (01), High Priestess (02), etc.

   **Minor Arcana (56 cards):**
   - Organize by suit in this order: Wands, Cups, Swords, Pentacles
   - Within each suit, use this naming convention:
     - Wands: `22.png` to `35.png`
     - Cups: `36.png` to `49.png`
     - Swords: `50.png` to `63.png`
     - Pentacles: `64.png` to `77.png`
   
   **Card sequence within each suit:**
   - Ace (1), 2, 3, 4, 5, 6, 7, 8, 9, 10, Page, Knight, Queen, King
   - Example: Ace of Wands = `22.png`, King of Pentacles = `77.png`

   **Card Back:**
   - Include a card back image named `back.png` (or .jpg/.jpeg)

3. **Create a folder** for your deck with a descriptive name
   - Example: `MyCustomDeck` or `ArtisticTarot`
   - Place all renamed card images in this folder

### Installing Your Custom Deck

#### For Standard Installations:

The location of the `decks` directory depends on how you installed TarotCaster:

1. **Linux:**
   - If installed from source: Look for a `decks` folder in the same directory as the TarotCaster executable

   2. **Windows:**
   - The `decks` folder is located in the TarotCaster installation directory

3. **macOS:**
   - The `decks` folder is inside the application package
   - Right-click on TarotCaster.app and select "Show Package Contents"
   - Navigate to `Contents/Resources/decks/`
   
  4. Copy your prepared deck folder into the appropriate `decks` directory
5. Restart TarotCaster - your deck will appear in the deck selection menu

#### For Flatpak Installations:

Flatpak apps run in a sandbox with limited access to the system. To add custom decks:

1. Create a `decks` directory in the Flatpak data folder:
   ```bash
   mkdir -p ~/.var/app/io.github.alamahant.TarotCaster/data/decks
   ```

2. Copy your prepared deck folder into this location:
   ```bash
   cp -r /path/to/MyCustomDeck ~/.var/app/io.github.alamahant.TarotCaster/data/decks/
   ```

3. Restart TarotCaster - your deck will be automatically detected and available in the deck selection menu

### Troubleshooting Custom Decks

- If your deck doesn't appear, check that all files are named correctly
- Ensure you have all 78 cards plus the back image
- Verify that the deck folder is in the correct location
- Check file permissions if using Linux or macOS
- For Flatpak, ensure the data directory path is correct

## Credits and Licensing

### Tarot Deck Artwork

#### Rider-Waite Tarot
- Original Rider-Waite Tarot deck artwork by Pamela Colman Smith under the direction of Arthur Edward Waite
- Images sourced from [Wikimedia Commons](https://commons.wikimedia.org/wiki/Category:Rider-Waite_tarot_deck)
- These images are in the public domain in the United States, the EU and globally because their copyright has expired

#### Tarot de Marseille
- Traditional Tarot de Marseille deck, a historical tarot pattern originating from southern France
- Images sourced from [Wikimedia Commons](https://commons.wikimedia.org/wiki/Category:Tarot_de_Marseille_(Single_Cards))
- These images are in the public domain due to their age and historical nature



### Tarot Interpretations
- Traditional card meanings adapted from [Corpora Project](https://github.com/dariusk/corpora/blob/master/data/divination/tarot_interpretations.json)
- AI interpretations powered by Mistral AI

## License

GPL-3.0 - See the LICENSE file for details

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
