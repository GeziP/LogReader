#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include "logexporter.h"
#include "appsettings.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QLabel;
QT_END_NAMESPACE

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    
    ExportConfig getExportConfig() const;
    void setLogCount(int count);

private slots:
    void onBrowseFileClicked();
    void onFormatChanged();
    void updatePreview();
    void onAccept();

private:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    QString getDefaultFileName() const;
    QString getFormatFilter() const;
    QStringList getSelectedFormats() const;
    QString getExtensionForFormat(const QString& format) const;
    void updateExportHint();
    
    // UI 组件
    QGroupBox *formatGroupBox;
    QCheckBox *txtCheckBox;
    QCheckBox *csvCheckBox;
    QCheckBox *jsonCheckBox;
    
    QGroupBox *fieldsGroupBox;
    QCheckBox *timestampCheckBox;
    QCheckBox *levelCheckBox;
    QCheckBox *moduleCheckBox;
    QCheckBox *contentCheckBox;
    
    QGroupBox *fileGroupBox;
    QLabel *exportHintLabel;
    QLineEdit *filePathLineEdit;
    QPushButton *browseButton;
    
    QLabel *previewLabel;
    QLabel *logCountLabel;
    
    QPushButton *okButton;
    QPushButton *cancelButton;
    
    int logCount;
};

#endif // EXPORTDIALOG_H 