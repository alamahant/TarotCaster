#include "helpdialog.h"

#include <QApplication>
#include <QStyle>
#include <QScreen>
#include"Globals.h"

HelpDialog::HelpDialog(DialogType type, QWidget *parent)
    : QDialog(parent),
    currentType(type)
{
    // Set up the dialog
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(600, 500);

    // Center the dialog on the screen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            QGuiApplication::primaryScreen()->availableGeometry()
            )
        );

    // Create layout
    mainLayout = new QVBoxLayout(this);

    // Create title label
    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // Create content browser
    contentBrowser = new QTextBrowser(this);
    contentBrowser->setOpenExternalLinks(true);
    contentBrowser->document()->setDefaultStyleSheet(
        "a { color: #d4af37; text-decoration: underline; }"
        "a:hover { color: #ffd700; }"
        );
    // Create close button
    closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // Add widgets to layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(contentBrowser, 1);
    mainLayout->addWidget(closeButton);

    // Set up content based on dialog type
    switch (type) {
    case About:
        setupAbout();
        break;
    case Instructions:
        setupInstructions();
        break;
    case Spreads:
        setupSpreads();
        break;
    case AddDecks:
        setupDecks();
        break;
    case CustomSpreads :
        setupCustomSpreads();
        break;
    case CustomSpreadHelp:
        setupCustomSpreadHelp();
        break;
    case ChangeLogHelp:
        setupChangelogHelp();
        break;
    }
}

void HelpDialog::setupAbout()
{
    setWindowTitle("About TaroCaster");
    titleLabel->setText("About TaroCaster");

    QString aboutText = QString(R"(
    <h2 align="center">TaroCaster v%1</h2>
    <p align="center">A modern tarot reading application with AI interpretation</p>
    <p align="center">© 2025 Alamahant</p>
    <hr>
    <p>TaroCaster is an open-source application that combines traditional tarot reading with
    modern AI interpretation capabilities.</p>
    <p>Features:</p>
    <ul>
        <li>Multiple tarot decks</li>
        <li>Various spread layouts</li>
        <li>AI-powered reading interpretations</li>
        <li>Save and load readings</li>
    </ul>
    <p>Built with Qt 6 and C++.</p>
    <p>Visit <a href="https://github.com/alamahant/TarotCaster">https://github.com/alamahant/TarotCaster</a> for more information.</p>
)").arg(QApplication::applicationVersion());



    contentBrowser->setHtml(aboutText);
}


void HelpDialog::setupInstructions()
{
    setWindowTitle("TaroCaster Instructions");
    titleLabel->setText("How to Use TaroCaster");

    QString instructionsText = R"(
        <h2 align="center">Getting Started with TaroCaster</h2>
        <hr>
        <h3>Basic Usage</h3>
        <ol>
            <li><strong>Select a Deck</strong> - Choose from available tarot decks in the dropdown menu.</li>
            <li><strong>Choose a Spread</strong> - Select the type of reading you want to perform.</li>
            <li><strong>Shuffle</strong> - Click the Shuffle button to randomize the cards.</li>
            <li><strong>Deal Cards</strong> - Click Deal Cards to lay out your spread.</li>
            <li><strong>Get Reading</strong> - Click Get Reading to receive an AI interpretation.</li>
        </ol>

        <h3>Interactive Shuffling</h3>
        <p>TaroCaster's unique shuffling system creates a personal connection with your reading:</p>
        <ol>
            <li>When you press "Shuffle", your cursor changes to a crosshair</li>
            <li>Move your cursor around the screen to gather entropy and infuse your energy</li>
            <li>This movement creates a unique shuffling pattern based on your personal interaction</li>
            <li>When ready, press "Stop Shuffling" to complete the process</li>
            <li>Then "Deal Cards" to lay out your spread</li>
        </ol>
        <p>This interactive approach creates a stronger connection between you and your reading,
        similar to physically shuffling a deck of cards.</p>

        <h3>AI Interpretation</h3>
        <p>TaroCaster uses Mistral AI's powerful language models to provide insightful tarot interpretations.
        To use this feature, you'll need a free Mistral API key:</p>
        <ol>
            <li>Visit <a href="https://console.mistral.ai/">https://console.mistral.ai/</a> to create a free account</li>
            <li>Generate an API key in your Mistral AI dashboard</li>
            <li>Enter the key in TaroCaster via Edit → Mistral API Key</li>
        </ol>
        <p>The free tier provides sufficient access for personal tarot readings.</p>

        <h3>Options</h3>
        <ul>
            <li><strong>Allow Reversed Cards</strong> - When checked, cards may appear upside-down, adding additional meanings.</li>
            <li><strong>Save/Load</strong> - Use the File menu to save readings for later reference.</li>
            <li><strong>API Key</strong> - Set your Mistral AI API key in Edit menu to enable AI interpretations.</li>
        </ul>

        <h3>Tips for Better Readings</h3>
        <ul>
            <li>Focus on a specific question or situation before shuffling.</li>
            <li>Take time to reflect on each card before requesting an interpretation.</li>
            <li>Remember that tarot is a tool for reflection and insight, not absolute prediction.</li>
        </ul>
    )";

    contentBrowser->setHtml(instructionsText);
}


void HelpDialog::setupSpreads()
{
    setWindowTitle("Tarot Spreads");
    titleLabel->setText("Available Tarot Spreads");

    QString spreadsText = R"(
        <h2 align="center">Tarot Spread Layouts</h2>
        <hr>

        <h3>Single Card</h3>
        <p>The simplest spread, drawing just one card to answer a specific question or provide insight for the day.</p>
        <p><strong>Position:</strong> One card representing the answer or insight.</p>
        <p>This spread is ideal for daily guidance, quick questions, or when you need a straightforward answer without the complexity of multiple cards interacting.</p>

        <h3>Three Card</h3>
        <p>A versatile spread that can represent past-present-future, mind-body-spirit, or situation-action-outcome.</p>
        <p><strong>Layout:</strong> Three cards in a horizontal row, read from left to right.</p>
        <p><strong>Positions:</strong></p>
        <ol>
            <li><strong>Past / Mind / Situation</strong> - The left card represents either past influences, your mental state, or the current situation depending on your chosen focus.</li>
            <li><strong>Present / Body / Action</strong> - The middle card represents either your current circumstances, physical reality, or actions you should consider.</li>
            <li><strong>Future / Spirit / Outcome</strong> - The right card represents either potential future developments, spiritual aspects, or the likely outcome of your situation.</li>
        </ol>
        <p>This spread is excellent for gaining perspective on a situation's development over time or understanding different aspects of a question.</p>

        <h3>Celtic Cross</h3>
        <p>A comprehensive 10-card spread that provides detailed insight into a situation. This classic spread offers a complete picture of influences surrounding your question.</p>
        <p><strong>Layout:</strong> The first six cards form a cross pattern, with cards 7-10 in a vertical line to the right.</p>
        <img src="qrc:/images/celtic_cross.svg" alt="Celtic Cross Layout" style="display: block; margin: 0 auto; max-width: 80%;">
        <p><strong>Positions:</strong></p>
        <ol>
            <li><strong>Present</strong> - The center of the cross, representing your current situation or the core issue.</li>
            <li><strong>Challenge</strong> - Placed horizontally across the first card, showing what crosses or challenges you. This card often represents an immediate obstacle.</li>
            <li><strong>Foundation</strong> - Below the center, showing the basis of the situation or underlying influences. This represents what's already established.</li>
            <li><strong>Recent Past</strong> - To the left of the center, showing recent events or influences that are still affecting the situation but beginning to fade.</li>
            <li><strong>Crown</strong> - Above the center, representing your hopes, goals, or the best possible outcome given the current circumstances.</li>
            <li><strong>Near Future</strong> - To the right of the center, showing upcoming influences or events that will soon come into play.</li>
            <li><strong>Self</strong> - The bottom card in the staff (vertical line to the right), representing your attitude, approach, or how you see yourself in this situation.</li>
            <li><strong>Environment</strong> - Second from bottom in the staff, showing external influences, other people's attitudes, or the environment surrounding the question.</li>
            <li><strong>Hopes/Fears</strong> - Third from bottom in the staff, revealing your expectations, what you're hoping for, or what you're afraid might happen.</li>
            <li><strong>Outcome</strong> - The top card in the staff, showing the final result or culmination of all the energies in the spread if the current course is maintained.</li>
        </ol>
        <p>The Celtic Cross provides a comprehensive view of your situation, examining it from multiple angles and timeframes. It's particularly useful for complex questions with many factors at play.</p>

        <h3>Horseshoe</h3>
        <p>A 7-card spread shaped like a horseshoe, focusing on a specific problem and its solution.</p>
        <p><strong>Layout:</strong> Cards are arranged in a horseshoe shape, starting from the bottom left, arcing upward, and ending at the bottom right.</p>
        <img src="qrc:/images/horseshoe.svg" alt="Horseshoe Layout" style="display: block; margin: 0 auto; max-width: 80%;">
        <p><strong>Positions:</strong></p>
        <ol>
            <li><strong>Past</strong> - Bottom left position, showing what led to the current situation. This card reveals the origins of your question or problem.</li>
            <li><strong>Present</strong> - Second position moving upward, representing the current situation and immediate circumstances you're dealing with.</li>
            <li><strong>Hidden Influences</strong> - Third position, revealing factors you may not be aware of that are affecting the situation. These can be subconscious motivations or external forces working behind the scenes.</li>
            <li><strong>Obstacles</strong> - Top center position (apex of the horseshoe), showing the main challenges or barriers you need to overcome. This is often the most important card in the spread.</li>
            <li><strong>External Influences</strong> - Fifth position moving downward, representing outside factors, other people's input, or environmental conditions affecting your situation.</li>
            <li><strong>Advice</strong> - Sixth position, providing guidance or suggesting actions to help resolve the situation. This card offers practical steps to consider.</li>
            <li><strong>Outcome</strong> - Bottom right position, showing the likely result if you follow the path indicated by the spread. This card reveals potential resolution.</li>
        </ol>
        <p>The Horseshoe spread is particularly effective for problem-solving, as it clearly identifies obstacles and provides specific advice for moving forward.</p>

        <h3>Zodiac Spread</h3>
        <p>A 12-card spread with each position representing a house of the zodiac, providing insight into different areas of life.</p>
        <p><strong>Layout:</strong> Cards are arranged in a circle, starting at the 9 o'clock position (representing the 1st house) and moving clockwise around the circle.</p>
        <img src="qrc:/images/zodiac.svg" alt="Zodiac Layout" style="display: block; margin: 0 auto; max-width: 80%;">
        <p><strong>Positions and House Meanings:</strong></p>
        <ol>
            <li><strong>1st House (Aries)</strong> - Self, identity, appearance, beginnings. This card reflects your personal approach to life and how you present yourself to the world.</li>
            <li><strong>2nd House (Taurus)</strong> - Values, possessions, resources, self-worth. This position reveals your relationship with material resources and personal values.</li>
            <li><strong>3rd House (Gemini)</strong> - Communication, siblings, local environment, learning. This card shows how you communicate and process information.</li>
            <li><strong>4th House (Cancer)</strong> - Home, family, roots, emotional foundation. This position reflects your inner emotional world and sense of belonging.</li>
            <li><strong>5th House (Leo)</strong> - Creativity, pleasure, children, self-expression. This card reveals how you express yourself creatively and find joy.</li>
            <li><strong>6th House (Virgo)</strong> - Health, daily routine, service, work. This position shows your approach to daily responsibilities and physical wellbeing.</li>
            <li><strong>7th House (Libra)</strong> - Partnerships, marriage, open enemies, contracts. This card reflects your one-on-one relationships and how you relate to others.</li>
            <li><strong>8th House (Scorpio)</strong> - Transformation, shared resources, intimacy, the occult. This position reveals deep psychological patterns and how you handle change.</li>
            <li><strong>9th House (Sagittarius)</strong> - Philosophy, higher education, travel, expansion. This card shows your worldview and quest for meaning.</li>
            <li><strong>10th House (Capricorn)</strong> - Career, public image, authority, achievement. This position reflects your ambitions and place in society.</li>
            <li><strong>11th House (Aquarius)</strong> - Friends, groups, hopes, humanitarian causes. This card shows your social connections and ideals for the future.</li>
            <li><strong>12th House (Pisces)</strong> - Unconscious, spirituality, hidden enemies, self-undoing. This position reveals your spiritual path and unconscious patterns.</li>
        </ol>
        <p>From a tarot perspective, the Zodiac spread provides a comprehensive life reading, touching on every major area of existence. Unlike other spreads that focus on a specific question, this spread gives a complete overview of your current life circumstances and potential.</p>
        <p>Each house position blends the energy of its corresponding zodiac sign with the meaning of the tarot card that falls there. For example, a Strength card in the 10th House would suggest career success through patience and inner fortitude, while The Tower in the 4th House might indicate dramatic changes in your home life or emotional foundation.</p>
        <p>This spread is ideal for annual readings, life path exploration, or when seeking guidance across multiple areas of life simultaneously.</p>
    )";

    contentBrowser->setHtml(spreadsText);
}

void HelpDialog::setupDecks() {
    setWindowTitle("Adding Custom Tarot Decks");
    titleLabel->setText("How to Add Additional Decks");

    QString decksDirPath = getUserDecksDirPath().toHtmlEscaped();

    QString decksText = QString(R"(
        <h2 align="center">Adding Your Own Tarot Decks</h2>
        <hr>

        <p style="color:red; font-weight:bold; font-size:1.1em;">
            Note: As of Version 1.2.1, users are encouraged to use the automated utility under <strong>Tools → Import Deck</strong>
            for easier deck installation and file renaming.
        </p>

        <h3>Preparing Your Deck Files</h3>
        <ol>
            <li><strong>Obtain digital images</strong> of your preferred tarot deck
                <ul>
                    <li>Ensure you have rights to use these images</li>
                    <li>PNG, JPG, and JPEG formats are supported</li>
                    <li>Consistent image dimensions are recommended</li>
                </ul>
            </li>
            <li><strong>Rename the files</strong> according to TarotCaster's required format:
                <ul>
                    <li><strong>Major Arcana (22 cards):</strong> Name files from <code>00.png</code> to <code>21.png</code> (or .jpg/.jpeg)</li>
                    <li><strong>Minor Arcana (56 cards):</strong> Organize by suit in this order:
                        <ul>
                            <li>Wands: <code>22.png</code> to <code>35.png</code></li>
                            <li>Cups: <code>36.png</code> to <code>49.png</code></li>
                            <li>Swords: <code>50.png</code> to <code>63.png</code></li>
                            <li>Pentacles: <code>64.png</code> to <code>77.png</code></li>
                        </ul>
                    </li>
                    <li><strong>Card Back:</strong> Include a card back image named <code>back.png</code> (or .jpg/.jpeg)</li>
                </ul>
            </li>
            <li><strong>Create a folder</strong> for your deck with a descriptive name
                <ul>
                    <li>Example: <code>MyCustomDeck</code> or <code>ArtisticTarot</code></li>
                    <li>Place all renamed card images in this folder</li>
                </ul>
            </li>
        </ol>

        <h3>Installing Your Custom Deck</h3>
        <p>Copy your prepared deck folder to the following directory:</p>
        <pre>%1</pre>
        <p>For example, if your deck is named <code>MyCustomDeck</code>, the full path would be:</p>
        <pre>%1/MyCustomDeck</pre>
        <p>Restart TarotCaster – your deck will be automatically detected and available in the deck selection menu.</p>

        <h3>Other Platform Paths</h3>
        <p>The default location for custom decks varies by operating system:</p>
        <ul>
            <li><strong>Linux (non-Flatpak):</strong>
                <pre>~/.local/share/TarotCaster/decks/</pre>
            </li>
            <li><strong>Windows:</strong>
                <pre>C:\Users\&lt;username&gt;\AppData\Local\TarotCaster\decks\</pre>
            </li>
            <li><strong>macOS:</strong>
                <pre>~/Library/Application Support/TarotCaster/decks/</pre>
            </li>
        </ul>
        <p>TarotCaster will automatically create these directories on first launch. Place your custom deck folder inside the appropriate directory for your system.</p>

        <h3>Card Sequence Details</h3>
        <p><strong>Major Arcana order:</strong> Fool (00), Magician (01), High Priestess (02), etc.</p>
        <p><strong>Card sequence within each suit:</strong> Ace (1), 2, 3, 4, 5, 6, 7, 8, 9, 10, Page, Knight, Queen, King</p>
        <p><strong>Example:</strong> Ace of Wands = <code>22.png</code>, King of Pentacles = <code>77.png</code></p>
    )").arg(decksDirPath, decksDirPath);

    contentBrowser->setHtml(decksText);
}


void HelpDialog::setupCustomSpreads() {
    setWindowTitle("Custom Spreads");
    titleLabel->setText("Description of your Custom Tarot Spreads");

    QString content = "<h2 align='center'>Custom Tarot Spreads</h2><hr>";

    if (!tarotScene) {
        content += "<p align='center'><i>Error: No tarot scene available.</i></p>";
        contentBrowser->setHtml(content);
        return;
    }

    // Get custom spreads directly from the tarotScene
    QVector<TarotScene::CustomSpread> customSpreads = tarotScene->getCustomSpreads();

    if (customSpreads.isEmpty()) {
        content += "<p align='center'><i>No custom spreads found.</i></p>";
        content += "<p align='center'>You can create custom spreads by using the 'Create Custom Spread' option.</p>";
    } else {
        for (const TarotScene::CustomSpread& spread : customSpreads) {
            // Create a section for each custom spread
            content += QString("<div style='margin-bottom: 30px;'>");

            // Spread name as header
            content += QString("<h3 style='margin-top:0; color:#8C6D46; border-bottom: 1px solid #8C6D46;'>%1</h3>")
                           .arg(spread.name);

            // Description
            if (!spread.description.isEmpty()) {
                content += QString("<p style='margin-bottom:15px;'>%1</p>")
                .arg(spread.description);
            }

            // Card positions
            content += "<h4 style='margin-bottom:5px; color:#8C6D46;'>Card Positions:</h4>";
            content += "<table style='border-collapse:collapse; width:100%;'>";

            for (const TarotScene::CustomSpreadPosition& pos : spread.positions) {
                content += QString("<tr><td style='padding:2px 5px; font-weight:bold; width:30px;'>%1.</td>"
                                   "<td style='padding:2px 5px; font-weight:bold;'>%2</td></tr>")
                               .arg(pos.number)
                               .arg(pos.name);

                content += QString("<tr><td></td><td style='padding:2px 5px; padding-bottom:8px;'>%1</td></tr>")
                               .arg(pos.significance);
            }

            content += "</table></div>";
        }
    }

    contentBrowser->setHtml(content);
}



void HelpDialog::setupCustomSpreadHelp() {
    setWindowTitle("Creating Custom Spreads");
    titleLabel->setText("How to Create Custom Tarot Spreads");

    QString content = R"(
        <h2 align='center'>Creating Your Own Tarot Spreads</h2>
        <hr>

        <p>The Custom Spread Designer allows you to create your own unique tarot spreads with personalized card positions and meanings.</p>

        <h3 style='color:#8C6D46;'>Getting Started</h3>
        <p>To create a custom spread, select "Create Custom Spread" from the Spreads menu. This will open the Custom Spread Designer dialog.</p>

        <h3 style='color:#8C6D46;'>Step 1: Define Your Spread</h3>
        <p>Start by entering basic information about your spread:</p>
        <ul>
            <li><strong>Spread Name:</strong> Give your spread a descriptive name.</li>
            <li><strong>Spread Description:</strong> Explain the purpose and meaning of your spread.</li>
        </ul>

        <h3 style='color:#8C6D46;'>Step 2: Add Card Positions</h3>
        <p>Click the "Add Card" button to add each position to your spread. For each position:</p>
        <ul>
            <li>A card will appear in the reading area</li>
            <li>Drag the card to position it where you want it in the spread</li>
            <li>The position will be added to the "Defined Positions" list</li>
        </ul>

        <h3 style='color:#8C6D46;'>Step 3: Define Each Position</h3>
        <p>Select each position in the "Defined Positions" list to edit its details:</p>
        <ul>
            <li><strong>Position Name:</strong> Give each position a meaningful name (e.g., "Past Influences", "Current Situation")</li>
            <li><strong>Position Significance:</strong> Explain what this position represents in your spread</li>
        </ul>
        <p>The selected card will be highlighted in the reading area, making it easy to identify which position you're editing.</p>

        <h3 style='color:#8C6D46;'>Step 4: Save Your Spread</h3>
        <p>Once you've added all positions and defined their meanings:</p>
        <ul>
            <li>Review your spread layout in the reading area</li>
            <li>Make sure all positions have names and significance descriptions</li>
            <li>Click "Save Spread" to store your custom spread</li>
        </ul>

        <h3 style='color:#8C6D46;'>Managing Custom Spreads</h3>
        <p>You can manage your saved custom spreads by clicking the "Manage Spreads" button in the Custom Spread Designer. This allows you to:</p>
        <ul>
            <li>View a list of all your custom spreads</li>
            <li>Delete spreads you no longer need</li>
        </ul>

        <h3 style='color:#8C6D46;'>Using Your Custom Spreads</h3>
        <p>Your custom spreads will appear in the Spreads menu alongside the built-in spreads. Select your custom spread to perform a reading with your personalized layout.</p>

        <p style='margin-top:20px;'><i>Note: Custom spreads are saved automatically and will be available the next time you open the application.</i></p>
    )";

    contentBrowser->setHtml(content);
}

void HelpDialog::setupChangelogHelp()
{
    setWindowTitle("Changelog");
    titleLabel->setText("TarotCaster Version History");
    QString content = R"(
        <h2 align='center'>Changelog</h2>
        <hr>

        <h3 style='color:#8C6D46;'>Version 1.2.1 - July 31, 2025</h3>

        <h4>New Features</h4>
        <ul>
            <li><strong>Sola Busca Tarot Deck Support:</strong>
                <ul>
                    <li>Added full support for the historical Sola Busca Tarot deck</li>
                    <li>Images sourced from Wikimedia Commons and rendered with high fidelity</li>
                    <li>Deck selectable from the deck manager alongside Rider-Waite and Marseille</li>
                </ul>
            </li>
            <li><strong>Universal Zoom Slider:</strong>
                <ul>
                    <li>Added a global zoom control slider to set overall card scaling</li>
                    <li>Affects all cards in the spread uniformly for personalized viewing comfort</li>
                </ul>
            </li>
            <li><strong>Strength–Justice Swap Option:</strong>
                <ul>
                    <li>Added checkbox setting to swap cards VIII and XI (Strength and Justice)</li>
                    <li>Supports historical deck orders like Tarot de Marseille</li>
                    <li>Swap applies dynamically per selected deck where applicable</li>
                </ul>
            </li>
            <li><strong>Dock Visibility Toggles:</strong>
                <ul>
                    <li>Under the <strong>View</strong> menu, users can now toggle visibility of left/right docks</li>
                    <li>Allows full focus on the card spread without UI distractions</li>
                </ul>
            </li>
            <li><strong>Deck Renaming Utility Dialog:</strong>
                <ul>
                    <li>New tool under 'Tools->Import Deck' to help users rename cards of custom digital decks</li>
                    <li>Automatically converts filenames to the naming convention used by TarotCaster</li>
                    <li>Ensures smooth import and compatibility with app features</li>
                </ul>
            </li>
        </ul>

        <h4>Visual and Technical Enhancements</h4>
        <ul>
            <li>Improved card rendering clarity with updated scaling and anti-aliasing techniques</li>
            <li>Enhanced sharpness and detail for all decks, especially at higher zoom levels</li>
            <li>Continued optimization of rendering pipeline for smoother performance</li>
            <li>General code cleanup and minor polish for better maintainability and stability</li>
        </ul>

        <hr>
        <h3 style='color:#8C6D46;'>Version 1.2.0 - May 16, 2025</h3>

        <h4>New Features</h4>
        <ul>
            <li><strong>Custom Spread Designer:</strong>
                <ul>
                    <li>Users can now create and save their own custom card layouts</li>
                    <li>Intuitive drag-and-drop interface for positioning cards</li>
                    <li>Option to name and describe the spiritual significance of each position</li>
                    <li>Save and load custom spreads for future readings</li>
                    <li>Share custom spreads with other users</li>
                </ul>
            </li>
            <li><strong>Card-specific Zoom Functionality:</strong>
                <ul>
                    <li>Users can now Ctrl+mouse wheel while hovering over a card to examine details</li>
                    <li>Zoom starts at the hover magnification (1.5x) and can increase up to 3x</li>
                    <li>Cards automatically return to normal size when no longer hovered</li>
                    <li>Zoom affects only the specific card being examined, not the entire spread</li>
                </ul>
            </li>
        </ul>

        <h4>Visual Enhancements</h4>
        <ul>
            <li>Optimized card rendering with pre-scaling for improved performance and image quality</li>
            <li>Maintained high-quality card hover effect with golden shadow highlighting</li>
            <li>Ensured smooth transitions between normal and highlighted card states</li>
            <li>Implemented OpenGL rendering for hardware-accelerated graphics performance</li>
        </ul>

        <h4>Technical Improvements</h4>
        <ul>
            <li>Implemented proper event handling for wheel events in card items</li>
            <li>Ensured compatibility between hover effects and zoom functionality</li>
            <li>Maintained visual consistency of shadow effects during zoom operations</li>
            <li>Optimized rendering with Qt::SmoothTransformation for high-quality card display</li>
            <li>Enabled OpenGL acceleration for smoother animations and effects</li>
            <li>Utilized GPU-accelerated compositing for shadow effects and transparency</li>
            <li>Added fallback rendering path for systems without OpenGL support</li>
            <li>Implemented pre-scaling system that scales all card images once during loading</li>
            <li>Added storage for pre-scaled images to eliminate redundant scaling operations</li>
        </ul>

        <h4>User Experience</h4>
        <ul>
            <li>Enhanced card examination capabilities while preserving the context of spreads</li>
            <li>Improved accessibility by allowing users to see card details more clearly</li>
            <li>Maintained intuitive interaction model with consistent visual feedback</li>
            <li>Significantly improved animation smoothness and overall responsiveness</li>
            <li>Reduced CPU usage during card animations and transitions</li>
        </ul>

        <h4>System Requirements</h4>
        <ul>
            <li>Added support for hardware acceleration on compatible systems</li>
            <li>Maintained compatibility with systems lacking dedicated graphics hardware</li>
        </ul>

        <hr>
        <h3 style='color:#8C6D46;'>Version 1.1.0 - April 29, 2025</h3>
        <!-- existing content unchanged below -->
    )";
    contentBrowser->setHtml(content);
}

