/**
 * @file logviewer.h
 * @brief Main Window Interface for LogReader Application
 * @details Defines the LogViewer class which serves as the primary user
 * interface for the log analysis application. Provides log file loading,
 * filtering, searching, and exporting capabilities through an intuitive
 * Qt-based UI.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QVector>
#include <QMainWindow>
#include <QStandardItem>

#include "../core/logentry.h"
#include "../core/logexporter.h"
#include "../utils/appsettings.h"
#include "../utils/languagemanager.h"

// Forward declarations to reduce compilation dependencies
QT_BEGIN_NAMESPACE
class QDateTimeEdit;
class QCheckBox;
class QComboBox;
class QTreeView;
class QStandardItemModel;
class QGroupBox;
class QVBoxLayout;
class QProgressBar;
class QAction;
class QPushButton;
class QLineEdit;
class QSplitter;
class QModelIndex;
class QHBoxLayout;
class QLabel;
class QMenu;
QT_END_NAMESPACE

class QTimer;

// Forward declarations for application classes (global namespace)
class LogTableModel;
class LogFilterProxyModel;
class HighlightDelegate;

/**
 * @class LogViewer
 * @brief Main application window for log file analysis and visualization
 * @details This class provides the primary user interface for the LogReader
 * application. It inherits from QMainWindow and offers comprehensive log
 * analysis features including file loading, multi-criteria filtering, real-time
 * searching, hierarchical display, and multi-format exporting.
 *
 * Key features:
 * - Log file loading with encoding detection
 * - Time range, level, and module-based filtering
 * - Real-time search with highlighting
 * - Hierarchical tree view for better organization
 * - Multi-format export (TXT, CSV, JSON)
 * - Language switching support
 * - Responsive UI with collapsible filter area
 *
 * The class follows the MVC pattern where LogEntry represents the model data,
 * QStandardItemModel serves as the data model for the tree view, and this class
 * acts as both the view and controller.
 *
 * Usage:
 * @code
 * LogViewer viewer;
 * viewer.show();
 * @endcode
 */
class LogViewer : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for LogViewer main window
     * @param parent Parent widget (typically nullptr for main window)
     * @details Initializes the main window, sets up the user interface,
     *          configures signal-slot connections, and applies saved settings.
     */
    LogViewer(QWidget* parent = nullptr);

protected:
    /**
     * @brief Event filter for handling mouse clicks on GitHub link
     * @param obj The object that received the event
     * @param event The event that occurred
     * @return true if the event was handled, false otherwise
     */
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    // File operations
    /**
     * @brief Handle file open action
     * @details Opens a file dialog for user to select log files (.log, .txt).
     *          Loads and parses the selected file with the chosen encoding.
     */
    void openLogFile();

    // Filtering operations
    /**
     * @brief Handle filter button click
     * @details Applies current filter criteria (time range, levels, modules)
     *          to the loaded log data and updates the tree view display.
     */
    void onFilterButtonClicked();

    /**
     * @brief Toggle visibility of filter control area
     * @details Shows/hides the filter panel to maximize viewing space.
     *          Updates the action text and icon accordingly.
     */
    void toggleFilterArea();

    /**
     * @brief Select all module checkboxes
     * @details Convenience function to quickly select all available log modules
     *          for filtering. Useful when user wants to see all modules.
     */
    void selectAllModules();

    /**
     * @brief Deselect all module checkboxes
     * @details Convenience function to quickly clear all module selections.
     *          Useful for starting fresh with module filtering.
     */
    void deselectAllModules();

    // Search operations
    /**
     * @brief Handle search text changes
     * @param text Current search text entered by user
     * @details Performs real-time search highlighting in the log tree view.
     *          Clears previous highlights and applies new ones for the search
     * term.
     */
    void onSearchTextChanged(const QString& text);

    /**
     * @brief Navigate to previous search result
     * @details Moves selection to the previous item containing the search term.
     *          Wraps around to the last result if at the beginning.
     */
    void onSearchPrevious();

    /**
     * @brief Navigate to next search result
     * @details Moves selection to the next item containing the search term.
     *          Wraps around to the first result if at the end.
     */
    void onSearchNext();

    /**
     * @brief Handle export search results action
     * @details Opens export dialog for user to choose format and options
     *          for exporting log entries that match the current search term.
     */
    void onExportSearchResults();

    // Display operations
    /**
     * @brief Handle double-click on log items
     * @param index Model index of the double-clicked item
     * @details Expands/collapses tree items or shows detailed information
     *          for individual log entries.
     */
    void onLogItemDoubleClicked(const QModelIndex& index);

    /**
     * @brief Handle tree item expansion
     * @param index Model index of the expanded item
     * @details Updates internal state when tree items are expanded,
     *          possibly loading additional data or updating view.
     */
    void onTreeItemExpanded(const QModelIndex& index);

    // Export operations
    /**
     * @brief Handle export filtered logs action
     * @details Opens export dialog for user to choose format and options
     *          for exporting currently filtered log data.
     */
    void onExportFiltered();

    // Language operations
    /**
     * @brief Handle language combo box selection change
     * @param index Selected index in the language combo box
     * @details Switches application language based on user selection
     *          and triggers UI refresh.
     */
    void onLanguageChanged(int index);

    /**
     * @brief Handle language manager change notifications
     * @param language New language that has been set
     * @details Updates UI elements when language is changed externally
     *          (e.g., through settings or other components).
     */
    void onLanguageManagerChanged(LanguageManager::Language language);

    // GitHub operations
    /**
     * @brief Handle GitHub link click
     * @details Opens the project's GitHub page in the default browser
     *          when the status bar GitHub link is clicked.
     */
    void onGitHubLinkClicked();

private:
    // Core functionality methods
    /**
     * @brief Load and parse a log file
     * @param filePath Path to the log file to load
     * @details Reads the log file with specified encoding, parses log entries,
     *          extracts unique modules and levels, and displays results in tree
     * view.
     */
    void loadLogFile(const QString& filePath);

    /**
     * @brief Set up the user interface
     * @details Creates and configures all UI elements including menus,
     * toolbars, filter controls, search controls, and the main log tree view.
     *          Establishes signal-slot connections between components.
     */
    void setupUI();

    /**
     * @brief Refresh UI text for current language
     * @details Updates all user-visible text elements to reflect current
     * language. Called when language is changed to provide immediate feedback.
     */
    void retranslateUI();

    /**
     * @brief Apply search highlighting to matching items
     * @details Highlights all tree items that contain the current search term,
     *          making them visually distinct for easy identification.
     */
    void highlightSearchMatches();

    /**
     * @brief Expand tree view to show specified item
     * @param item Tree item to make visible
     * @details Expands all parent items necessary to show the specified item,
     *          ensuring search results are visible to the user.
     */
    void expandToItem(QStandardItem* item);

    /**
     * @brief Get currently filtered log entries
     * @return List of log entries currently displayed in tree view
     * @details Extracts log entries from the current tree view model,
     *          useful for export operations.
     */
    QList<LogEntry> getCurrentFilteredLogs() const;

    /**
     * @brief Get log entries matching current search term
     * @return List of log entries that contain the current search text
     * @details Extracts log entries from searchResults indices,
     *          useful for exporting search matches.
     */
    QList<LogEntry> getSearchMatchedLogs() const;

    // UI Controls - Filter area
    QDateTimeEdit*
        startTimeEdit; ///< Start time selector for time range filtering
    QDateTimeEdit* endTimeEdit; ///< End time selector for time range filtering
    QList<QCheckBox*> levelCheckBoxes;  ///< Checkboxes for log level selection
    QList<QCheckBox*> moduleCheckBoxes; ///< Checkboxes for module selection
    QComboBox* encodingComboBox; ///< Dropdown for file encoding selection
    QComboBox* languageComboBox; ///< Dropdown for language selection

    // UI Controls - Main display
    QTreeView* logTreeView;       ///< Main tree view for displaying logs
    // Replaced heavy item model with lightweight table + proxy
    class LogTableModel* sourceModel; ///< Lightweight source model
    class LogFilterProxyModel* proxyModel; ///< Filter proxy model
    class HighlightDelegate* highlightDelegate; ///< Delegate for search highlight

    // UI Controls - Layout containers
    QGroupBox* timeGroupBox;   ///< Container for time range controls
    QGroupBox* levelGroupBox;  ///< Container for level selection controls
    QGroupBox* moduleGroupBox; ///< Container for module selection controls
    QHBoxLayout* moduleLayout; ///< Layout for module checkboxes
    QProgressBar* progressBar; ///< Progress indicator for long operations
    QWidget* filterWidget;     ///< Container for all filter controls
    QSplitter* mainSplitter;   ///< Splitter between filter area and main view

    // UI Controls - Actions and buttons
    QAction* openAction;         ///< File menu open action
    QAction* filterAction;       ///< Filter action for toolbar
    QAction* toggleFilterAction; ///< Action to toggle filter area visibility
    QAction* exportAction;       ///< Export action for menu and toolbar
    QPushButton* selectAllModulesButton;   ///< Button to select all modules
    QPushButton* deselectAllModulesButton; ///< Button to deselect all modules

    // UI Controls - Search
    QLineEdit* searchLineEdit;     ///< Text input for search terms
    QPushButton*
        searchPreviousButton;      ///< Button to go to previous search result
    QPushButton* searchNextButton; ///< Button to go to next search result
    QPushButton* exportSearchResultsButton; ///< Button to export search results

    // UI Controls - Retranslatable labels
    QLabel* startTimeLabel = nullptr;  ///< Label for start time selector
    QLabel* endTimeLabel = nullptr;    ///< Label for end time selector
    QLabel* encodingLabel = nullptr;   ///< Label for encoding selector
    QLabel* languageLabel = nullptr;   ///< Label for language selector

    // UI Controls - Menu
    QMenu* fileMenu = nullptr;  ///< File menu for retranslation

    // UI Controls - GitHub link
    QLabel* githubLinkLabel; ///< Clickable GitHub link in status bar

    // Data storage
    QStringList allModules;    ///< List of all unique modules found in logs
    QStringList allLevels;     ///< List of all unique log levels found
    QString currentFilePath;   ///< Path of currently loaded log file
    QString currentSearchText; ///< Current search term
    QVector<int> searchResults; ///< Row indices in proxy model matching search
    int currentSearchIndex;    ///< Index of currently selected search result
    QFont logFont;             ///< Font used for displaying log content
    QTimer* searchDebounceTimer; ///< Debounce timer for search input
};

#endif // LOGVIEWER_H
