/**
 * @file exportdialog.h
 * @brief Export Configuration Dialog Interface
 * @details Provides a user-friendly dialog interface for configuring log export
 *          operations including format selection, field inclusion, and file path
 *          specification. Integrates with the LogExporter system for seamless
 *          multi-format export functionality.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include "../core/logexporter.h"
#include "../utils/appsettings.h"

// Forward declarations to reduce compilation dependencies
QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QLabel;
QT_END_NAMESPACE

/**
 * @class ExportDialog
 * @brief User interface dialog for configuring log export operations
 * @details This dialog provides a comprehensive interface for users to configure
 *          log export parameters including target formats, field inclusion settings,
 *          and output file paths. It integrates with the AppSettings system to
 *          remember user preferences and provides real-time preview of export
 *          configurations.
 * 
 * Key features:
 * - Multi-format selection (TXT, CSV, JSON)
 * - Selective field inclusion/exclusion
 * - File path browsing with smart defaults
 * - Real-time export preview
 * - Settings persistence across sessions
 * - Format-specific file name suggestions
 * - Export statistics display
 * 
 * The dialog follows standard Qt dialog patterns with OK/Cancel buttons
 * and immediate validation of user inputs. It communicates with the
 * LogExporter system through the ExportConfig structure.
 * 
 * Usage example:
 * @code
 * ExportDialog dialog(this);
 * dialog.setLogCount(filteredLogs.size());
 * if (dialog.exec() == QDialog::Accepted) {
 *     ExportConfig config = dialog.getExportConfig();
 *     // Use config for export operation
 * }
 * @endcode
 */
class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for ExportDialog
     * @param parent Parent widget for modal dialog behavior (optional)
     * @details Initializes the export configuration dialog, sets up the user
     *          interface, loads saved settings, and configures signal connections.
     *          The dialog is modal by default when a parent is specified.
     */
    explicit ExportDialog(QWidget *parent = nullptr);
    
    /**
     * @brief Get the current export configuration
     * @return ExportConfig structure with user-selected options
     * @details Retrieves the complete export configuration based on current
     *          dialog settings including selected formats, field inclusions,
     *          and target file path. This configuration can be directly used
     *          with LogExporter for export operations.
     */
    ExportConfig getExportConfig() const;
    
    /**
     * @brief Set the number of log entries to be exported
     * @param count Number of log entries that will be exported
     * @details Updates the dialog to display the number of log entries that
     *          will be exported, helping users understand the scope of the
     *          export operation. Also updates export hints and file size estimates.
     */
    void setLogCount(int count);

private slots:
    /**
     * @brief Handle file path browse button click
     * @details Opens a file dialog for users to select or specify the target
     *          export file path. Automatically applies appropriate file filters
     *          based on selected export formats and updates the file path field.
     */
    void onBrowseFileClicked();
    
    /**
     * @brief Handle changes in format selection checkboxes
     * @details Responds to changes in export format selection by updating
     *          file path suggestions, format-specific hints, and enabling/disabling
     *          related UI elements. Ensures consistent UI state.
     */
    void onFormatChanged();
    
    /**
     * @brief Update the export configuration preview
     * @details Refreshes the preview display showing current export settings
     *          including selected formats, included fields, target file paths,
     *          and estimated export statistics. Called whenever settings change.
     */
    void updatePreview();
    
    /**
     * @brief Handle dialog acceptance (OK button)
     * @details Validates current settings, saves user preferences to persistent
     *          storage, and accepts the dialog if all configurations are valid.
     *          Shows error messages for invalid configurations.
     */
    void onAccept();

private:
    /**
     * @brief Set up the user interface layout and components
     * @details Creates and arranges all UI elements including format selection
     *          checkboxes, field inclusion options, file path controls, and
     *          preview displays. Establishes the overall dialog layout.
     */
    void setupUI();
    
    /**
     * @brief Set up signal-slot connections between UI components
     * @details Connects UI element signals to appropriate slot functions to
     *          ensure responsive behavior when users interact with the dialog.
     *          Includes format change notifications and validation triggers.
     */
    void setupConnections();
    
    /**
     * @brief Load saved export preferences from settings
     * @details Retrieves previously saved user preferences including default
     *          formats, field selections, and file paths from AppSettings.
     *          Applies these settings to initialize the dialog state.
     */
    void loadSettings();
    
    /**
     * @brief Save current export preferences to settings  
     * @details Stores current dialog settings to persistent storage through
     *          AppSettings for automatic restoration in future dialog sessions.
     *          Called when dialog is accepted.
     */
    void saveSettings();
    
    /**
     * @brief Generate a default filename based on current timestamp
     * @return Default filename string without path or extension
     * @details Creates a meaningful default filename incorporating current
     *          date and time to avoid naming conflicts and provide context
     *          for exported files.
     */
    QString getDefaultFileName() const;
    
    /**
     * @brief Get file dialog filter string for selected formats
     * @return File filter string for QFileDialog
     * @details Constructs an appropriate file filter string based on currently
     *          selected export formats for use in file browse dialogs.
     *          Supports multi-format selection.
     */
    QString getFormatFilter() const;
    
    /**
     * @brief Get list of currently selected export formats
     * @return List of format names that are currently selected
     * @details Examines format checkboxes and returns a list of format names
     *          that are currently selected by the user. Used for configuration
     *          building and validation.
     */
    QStringList getSelectedFormats() const;
    
    /**
     * @brief Get file extension for a specific format name
     * @param format Format name string
     * @return File extension string including the dot (e.g., ".csv")
     * @details Maps format names to their corresponding file extensions
     *          for automatic file naming and filter generation.
     */
    QString getExtensionForFormat(const QString& format) const;
    
    /**
     * @brief Update export operation hints and tips
     * @details Updates informational text that guides users about the current
     *          export configuration, including file naming conventions,
     *          format-specific notes, and operation estimates.
     */
    void updateExportHint();
    
    // UI Components - Format Selection
    QGroupBox *formatGroupBox;     ///< Container for export format selection controls
    QCheckBox *txtCheckBox;        ///< Checkbox for TXT format selection
    QCheckBox *csvCheckBox;        ///< Checkbox for CSV format selection  
    QCheckBox *jsonCheckBox;       ///< Checkbox for JSON format selection
    
    // UI Components - Field Selection
    QGroupBox *fieldsGroupBox;     ///< Container for field inclusion controls
    QCheckBox *timestampCheckBox;  ///< Checkbox for timestamp field inclusion
    QCheckBox *levelCheckBox;      ///< Checkbox for log level field inclusion
    QCheckBox *moduleCheckBox;     ///< Checkbox for module field inclusion
    QCheckBox *contentCheckBox;    ///< Checkbox for content field inclusion
    
    // UI Components - File Path
    QGroupBox *fileGroupBox;       ///< Container for file path controls
    QLabel *exportHintLabel;       ///< Label showing export hints and information
    QLineEdit *filePathLineEdit;   ///< Text field for target file path input
    QPushButton *browseButton;     ///< Button to open file browse dialog
    
    // UI Components - Information Display
    QLabel *previewLabel;          ///< Label showing export configuration preview
    QLabel *logCountLabel;         ///< Label showing number of entries to export
    
    // UI Components - Dialog Controls
    QPushButton *okButton;         ///< OK button to accept dialog
    QPushButton *cancelButton;     ///< Cancel button to reject dialog
    
    // Data Members
    int logCount;                  ///< Number of log entries to be exported
};

#endif // EXPORTDIALOG_H 