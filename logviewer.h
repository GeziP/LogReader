#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QMainWindow>
#include <QDateTime>
#include <QList>
#include <QStandardItem>
#include "logentry.h"
#include "logexporter.h"
#include "appsettings.h"

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
QT_END_NAMESPACE

class LogViewer : public QMainWindow
{
    Q_OBJECT
public:
    LogViewer(QWidget *parent = nullptr);
private slots:
    void onFilterButtonClicked();
    void openLogFile();
    void toggleFilterArea();
    void selectAllModules();
    void deselectAllModules();
    void onSearchTextChanged(const QString &text);
    void onSearchPrevious();
    void onSearchNext();
    void onLogItemDoubleClicked(const QModelIndex &index);
    void onTreeItemExpanded(const QModelIndex &index);
    void onExportFiltered();
private:
    void loadLogFile(const QString& filePath);
    void setupUI();
    void displayLogs(const QList<LogEntry>& logs);
    QList<LogEntry> parseLogFile(const QString& filePath, const QString& encoding);
    QList<LogEntry> filterLogs(const QList<LogEntry>& logs,
                               const QDateTime& startTime,
                               const QDateTime& endTime,
                               const QStringList& levels,
                               const QStringList& modules);
    void searchInItem(QStandardItem *item);
    void clearSearchHighlights();
    void clearHighlightsInItem(QStandardItem *item);
    void highlightSearchMatches();
    void expandToItem(QStandardItem *item);
    QList<LogEntry> getCurrentFilteredLogs() const;
    // 界面控件
    QDateTimeEdit *startTimeEdit;
    QDateTimeEdit *endTimeEdit;
    QList<QCheckBox*> levelCheckBoxes;
    QList<QCheckBox*> moduleCheckBoxes;
    QComboBox *encodingComboBox;
    QTreeView *logTreeView;
    QStandardItemModel *logModel;
    QGroupBox *timeGroupBox;
    QGroupBox *levelGroupBox;
    QGroupBox *moduleGroupBox;
    QHBoxLayout *moduleLayout;
    QProgressBar *progressBar;
    QAction *openAction;
    QAction *filterAction;
    QAction *toggleFilterAction;
    QAction *exportAction;
    QWidget *filterWidget;
    QPushButton *selectAllModulesButton;
    QPushButton *deselectAllModulesButton;
    QLineEdit *searchLineEdit;
    QPushButton *searchPreviousButton;
    QPushButton *searchNextButton;
    QSplitter *mainSplitter;
    // 数据
    QList<LogEntry> allLogs;
    QStringList allModules;
    QStringList allLevels;
    QString currentFilePath;
    QString currentSearchText;
    QList<QStandardItem*> searchResults;
    int currentSearchIndex;
    QFont logFont;
};

#endif // LOGVIEWER_H
