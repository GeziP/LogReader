/**
 * @file logviewer.cpp
 * @brief Implementation of Main Window Interface for LogReader Application
 * @details Contains the complete implementation of LogViewer class methods
 * including UI setup, log file processing, filtering, searching, and export
 * functionality. This is the core implementation file that brings together all
 * application features into a cohesive user interface.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#include "logviewer.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QStandardItemModel>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "exportdialog.h"
#include "formattemplatedialog.h"
#include "highlightdelegate.h"
#include "logfilterproxymodel.h"
#include "logtablemodel.h"

#include "../core/logloader.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
#include <QBrush>
#include <QDebug>
#include <QDesktopServices>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QStyleFactory>
#include <QStyledItemDelegate>
#include <QSysInfo>
#include <QUrl>
#include <utility>

/**
 * @brief Constructor for LogViewer main window
 * @param parent Parent widget for window management
 * @details Initializes the main application window, sets up the complete user
 * interface, configures signal-slot connections, and establishes the language
 * change callback. This is the primary entry point for the application's main
 * functionality.
 */
LogViewer::LogViewer(QWidget* parent)
    : QMainWindow(parent),
      sourceModel(nullptr),
      proxyModel(nullptr),
      highlightDelegate(nullptr),
      currentSearchIndex(-1),
      searchDebounceTimer(nullptr)
{
    setupUI();

    // Connect language manager signal for dynamic UI updates
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged,
            this, &LogViewer::onLanguageManagerChanged);
}

/**
 * @brief Set up the complete user interface layout and styling
 * @details Creates and configures all UI elements including:
 *          - Window title and icon
 *          - Application-wide styling with Fusion theme
 *          - Font configuration
 *          - Filter area with time range, level, and module selection
 *          - Main log tree view with custom styling
 *          - Search functionality
 *          - Menu bar and toolbar
 *          - Status bar and progress indicators
 */
void LogViewer::setupUI()
{
    // Set window title and icon
    setWindowTitle(tr("日志查看器"));
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    // Configure application-wide styling with modern Fusion theme
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // Set global font configuration for consistent appearance
    QFont defaultFont = this->font();
    defaultFont.setPointSize(10);
    this->setFont(defaultFont);

    // Initialize log display font
    logFont = defaultFont;

    // Apply custom stylesheet for professional appearance
    QString qss = R"(
    QGroupBox {
        font-weight: bold;
        border: 1px solid gray;
        border-radius: 5px;
        margin-top: 1ex;
    }

    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top center;
        padding: 0 3px;
    }

    QPushButton {
        background-color: #4CAF50;
        color: white;
        border-radius: 5px;
        padding: 5px;
    }

    QPushButton:hover {
        background-color: #45a049;
    }

    QTreeView {
        gridline-color: #ddd;
        selection-background-color: #3399FF; /* Selected row background color */
        selection-color: white; /* Selected row text color */
    }

    QTreeView::item {
        background-color: white;
        color: black;
    }

    QTreeView::item:selected {
        background-color: #3399FF; /* Selected row background color */
        color: white; /* Selected row text color */
    }

    QHeaderView::section {
        background-color: #f3f3f3;
        padding: 4px;
        border: none;
        font-weight: bold;
    }

    QCheckBox {
        spacing: 5px;
    }
    )";

    this->setStyleSheet(qss);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create filter area container
    filterWidget = new QWidget(this);

    // Time range selection controls
    startTimeLabel = new QLabel(tr("开始时间:"), this);
    startTimeEdit = new QDateTimeEdit(this);
    startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    endTimeLabel = new QLabel(tr("结束时间:"), this);
    endTimeEdit = new QDateTimeEdit(this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    QHBoxLayout* timeLayout = new QHBoxLayout();
    timeLayout->addWidget(startTimeLabel);
    timeLayout->addWidget(startTimeEdit);
    timeLayout->addWidget(endTimeLabel);
    timeLayout->addWidget(endTimeEdit);
    timeLayout->addStretch();

    timeGroupBox = new QGroupBox(tr("时间范围"), this);
    timeGroupBox->setLayout(timeLayout);

    // Log level selection controls
    levelGroupBox = new QGroupBox(tr("日志等级"), this);
    QHBoxLayout* levelLayout = new QHBoxLayout();
    levelGroupBox->setLayout(levelLayout);

    // Create log level checkboxes with sorted order
    QStringList levels = {"DEBUG", "ERROR", "INFO", "WARN"};
    levels.sort(Qt::CaseInsensitive); // Sort alphabetically

    for (const QString& level : levels) {
        QCheckBox* checkBox = new QCheckBox(level, this);
        checkBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        levelCheckBoxes.append(checkBox);
        levelLayout->addWidget(checkBox);
    }

    // Add flexible space for left alignment
    levelLayout->addStretch();

    // Module selection controls
    moduleLayout = new QHBoxLayout();

    // Add "Select All" and "Deselect All" buttons for module selection
    selectAllModulesButton = new QPushButton(tr("全选"), this);
    deselectAllModulesButton = new QPushButton(tr("全不选"), this);
    connect(selectAllModulesButton, &QPushButton::clicked, this,
            &LogViewer::selectAllModules);
    connect(deselectAllModulesButton, &QPushButton::clicked, this,
            &LogViewer::deselectAllModules);

    QHBoxLayout* moduleButtonLayout = new QHBoxLayout();
    moduleButtonLayout->addWidget(selectAllModulesButton);
    moduleButtonLayout->addWidget(deselectAllModulesButton);
    moduleButtonLayout->addStretch();

    // Add scroll area to support cases with many modules
    QScrollArea* moduleScrollArea = new QScrollArea(this);
    QWidget* moduleContainer = new QWidget(this);
    moduleLayout->setContentsMargins(5, 5, 5, 5);
    moduleContainer->setLayout(moduleLayout);
    moduleScrollArea->setWidgetResizable(true);
    moduleScrollArea->setWidget(moduleContainer);

    // Configure horizontal scrolling for module selection
    moduleScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    moduleScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set fixed height to prevent vertical scrollbar
    moduleScrollArea->setFixedHeight(60); // Adjust height as needed

    moduleGroupBox = new QGroupBox(tr("模块选择"), this);
    QVBoxLayout* moduleGroupLayout = new QVBoxLayout();
    moduleGroupLayout->addLayout(moduleButtonLayout); // Add button layout
    moduleGroupLayout->addWidget(moduleScrollArea);
    moduleGroupBox->setLayout(moduleGroupLayout);

    // Encoding selection
    encodingLabel = new QLabel(tr("文件编码:"), this);
    encodingComboBox = new QComboBox(this);
    encodingComboBox->addItems({"UTF-8", "GB18030", "GB2312"});
    encodingComboBox->setCurrentText("UTF-8");

    QHBoxLayout* encodingLayout = new QHBoxLayout();
    encodingLayout->addWidget(encodingLabel);
    encodingLayout->addWidget(encodingComboBox);
    encodingLayout->addStretch();

    // Filter button
    QPushButton* filterButton = new QPushButton(tr("筛选"), this);
    connect(filterButton, &QPushButton::clicked, this,
            &LogViewer::onFilterButtonClicked);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addStretch();
    filterLayout->addWidget(filterButton);

    // Combine filter area
    QVBoxLayout* filterAreaLayout = new QVBoxLayout();
    filterAreaLayout->addWidget(timeGroupBox);
    filterAreaLayout->addWidget(levelGroupBox);
    filterAreaLayout->addWidget(moduleGroupBox);

    // Extra fields filter container (populated dynamically)
    extraFieldsLayout = new QVBoxLayout();

    filterAreaLayout->addLayout(extraFieldsLayout);
    filterAreaLayout->addLayout(encodingLayout);
    filterAreaLayout->addLayout(filterLayout);
    filterWidget->setLayout(filterAreaLayout);

    // Log tree view
    logTreeView = new QTreeView(this);
    logTreeView->setAlternatingRowColors(true);
    logTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTreeView->setUniformRowHeights(true);
    logTreeView->setHeaderHidden(false);
    logTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    logTreeView->setFont(logFont); // Set initial font

    // Add the following two lines
    logTreeView->setItemsExpandable(true);
    logTreeView->setRootIsDecorated(true);

    // Set horizontal scrollbar
    logTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Connect double-click signal
    connect(logTreeView, &QTreeView::doubleClicked, this,
            &LogViewer::onLogItemDoubleClicked);
    // In setupUI or constructor, connect expand signal
    connect(logTreeView, &QTreeView::expanded, this,
            &LogViewer::onTreeItemExpanded);

    // Search line edit and buttons
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("搜索..."));
    connect(searchLineEdit, &QLineEdit::textChanged, this,
            &LogViewer::onSearchTextChanged);

    // Search debounce timer to avoid re-scanning on every keystroke
    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    searchDebounceTimer->setInterval(200);
    connect(searchDebounceTimer, &QTimer::timeout, this,
            &LogViewer::highlightSearchMatches);

    searchPreviousButton = new QPushButton(tr("上一条"), this);
    searchNextButton = new QPushButton(tr("下一条"), this);
    exportSearchResultsButton = new QPushButton(tr("导出搜索结果"), this);
    connect(searchPreviousButton, &QPushButton::clicked, this,
            &LogViewer::onSearchPrevious);
    connect(searchNextButton, &QPushButton::clicked, this,
            &LogViewer::onSearchNext);
    connect(exportSearchResultsButton, &QPushButton::clicked, this,
            &LogViewer::onExportSearchResults);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchPreviousButton);
    searchLayout->addWidget(searchNextButton);
    searchLayout->addWidget(exportSearchResultsButton);
    searchLayout->addStretch();

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Use QSplitter to split filter area and log view
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(filterWidget);
    mainSplitter->addWidget(logTreeView);
    mainSplitter->setStretchFactor(1, 1); // Let log view take more space

    mainLayout->addWidget(mainSplitter);
    mainLayout->addLayout(searchLayout);

    centralWidget->setLayout(mainLayout);

    // Menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    fileMenu = menuBar->addMenu(tr("文件"));
    openAction =
        fileMenu->addAction(QIcon(":/icons/open.png"), tr("打开日志文件"));
    connect(openAction, &QAction::triggered, this, &LogViewer::openLogFile);
    fileMenu->addSeparator();
    exportAction =
        fileMenu->addAction(QIcon(":/icons/export.png"), tr("导出筛选结果"));
    exportAction->setEnabled(false); // Initially disabled
    connect(exportAction, &QAction::triggered, this,
            &LogViewer::onExportFiltered);
    setMenuBar(menuBar);

    // Tool bar
    QToolBar* toolBar = addToolBar(tr("工具栏"));
    toolBar->addAction(openAction);
    toolBar->addAction(exportAction);
    filterAction = toolBar->addAction(QIcon(":/icons/filter.png"), tr("筛选"));
    connect(filterAction, &QAction::triggered, this,
            &LogViewer::onFilterButtonClicked);

    // Show/hide filter area button
    toggleFilterAction = toolBar->addAction(QIcon(":/icons/toggle.png"),
                                            tr("显示/隐藏筛选区域"));
    connect(toggleFilterAction, &QAction::triggered, this,
            &LogViewer::toggleFilterArea);

    // Format template button
    formatTemplateAction =
        toolBar->addAction(QIcon(":/icons/template.png"), tr("日志格式"));
    connect(formatTemplateAction, &QAction::triggered, this,
            &LogViewer::onFormatTemplateAction);

    // Add separator
    toolBar->addSeparator();

    // Language switch widget
    languageLabel = new QLabel(tr("语言:"), this);
    languageComboBox = new QComboBox(this);

    // Initialize language options
    QMap<LanguageManager::Language, QString> supportedLanguages =
        LanguageManager::instance().getSupportedLanguages();
    for (auto it = supportedLanguages.constBegin();
         it != supportedLanguages.constEnd(); ++it) {
        languageComboBox->addItem(it.value(), static_cast<int>(it.key()));
    }

    // Set current language
    LanguageManager::Language currentLang =
        LanguageManager::instance().getCurrentLanguage();
    int currentIndex =
        languageComboBox->findData(static_cast<int>(currentLang));
    if (currentIndex >= 0) {
        languageComboBox->setCurrentIndex(currentIndex);
    }

    // Connect signal
    connect(languageComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &LogViewer::onLanguageChanged);

    toolBar->addWidget(languageLabel);
    toolBar->addWidget(languageComboBox);

    // Status bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    statusBar()->addPermanentWidget(progressBar);

    // Create GitHub link label
    githubLinkLabel = new QLabel(this);
    githubLinkLabel->setText("⭐ GitHub");
    githubLinkLabel->setStyleSheet(
        "color: #0969da; text-decoration: underline; padding: "
        "2px 6px;");
    githubLinkLabel->setCursor(Qt::PointingHandCursor);
    githubLinkLabel->setToolTip(tr("点击访问GitHub项目页面并给我们点赞"));
    githubLinkLabel->installEventFilter(this);
    statusBar()->addPermanentWidget(githubLinkLabel);

    // 显示运行时Qt版本与构建信息，便于排查发布差异
    QString buildInfo = QString("Qt %1 | %2 | %3")
                            .arg(QString::fromLatin1(qVersion()))
                            .arg(QString::fromLatin1(QT_VERSION_STR))
                            .arg(QString::fromLatin1(
#ifdef NDEBUG
                                "Release"
#else
                                "Debug"
#endif
                                ));
    statusBar()->showMessage(tr("就绪 · %1").arg(buildInfo));

    // Initialize models
    sourceModel = new LogTableModel(this);
    proxyModel = new LogFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);
    logTreeView->setModel(proxyModel);
    // Delegate for highlight on content column
    highlightDelegate = new HighlightDelegate(this);
    logTreeView->setItemDelegateForColumn(LogTableModel::ColumnMessage,
                                          highlightDelegate);
}

void LogViewer::openLogFile()
{
    // Use remembered directory as default path
    QString defaultDir = AppSettings::instance().getRecentLogDir();
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("打开日志文件"), defaultDir,
        tr("Log Files (*.log *.txt);;All Files (*)"));
    if (!filePath.isEmpty()) {
        loadLogFile(filePath);
        // Save new directory path
        QFileInfo fileInfo(filePath);
        AppSettings::instance().setRecentLogDir(fileInfo.absolutePath());
    }
}

void LogViewer::loadLogFile(const QString& filePath)
{
    currentFilePath = filePath;
    QString encoding = encodingComboBox->currentText();

    // Show progress bar
    progressBar->setVisible(true);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    // Clear previous data
    sourceModel->clear();
    // Reset dynamic columns from previous file
    sourceModel->setExtraColumns(QStringList());
    // Clear extra field filter UI
    QLayoutItem* child;
    while ((child = extraFieldsLayout->takeAt(0)) != nullptr) {
        QWidget* widget = child->widget();
        if (widget)
            widget->deleteLater();
        delete child;
    }
    extraFieldCheckBoxes.clear();

    // Background loader
    QThread* thread = new QThread(this);
    QString formatTemplate = AppSettings::instance().getLogFormatTemplate();
    LogLoader* loader = new LogLoader(filePath, encoding, 5000, formatTemplate);
    loader->moveToThread(thread);

    connect(thread, &QThread::started, loader, &LogLoader::process);
    // Compatibility: some Qt signals might differ; ensure progress() is used
    connect(loader, &LogLoader::progress, progressBar, &QProgressBar::setValue);
    connect(loader, &LogLoader::error, this, [this](const QString& msg) {
        QMessageBox::warning(this, tr("错误"), msg);
    });
    // 发布构建下，加载期间冻结视图更新以减少重绘
#ifdef NDEBUG
    logTreeView->setUpdatesEnabled(false);
    logTreeView->viewport()->setUpdatesEnabled(false);
#endif

    connect(loader, &LogLoader::chunkReady, this,
            [this](QVector<LogEntry> chunk) {
                sourceModel->appendRows(std::move(chunk));
            });
    connect(loader, &LogLoader::summaryReady, this,
            [this](const QDateTime& minTime, const QDateTime& maxTime,
                   const QStringList& modules, const QStringList& levels,
                   const QStringList& extraFieldNames,
                   const QMap<QString, QStringList>& extraFieldValues) {
                startTimeEdit->setDateTime(minTime);
                endTimeEdit->setDateTime(maxTime);
                allModules = modules;
                allLevels = levels;
                // Rebuild module checkboxes UI
                QLayoutItem* child;
                while ((child = moduleLayout->takeAt(0)) != nullptr) {
                    QWidget* widget = child->widget();
                    if (widget)
                        widget->deleteLater();
                    delete child;
                }
                moduleCheckBoxes.clear();
                for (const QString& module : allModules) {
                    QCheckBox* checkBox = new QCheckBox(module, this);
                    checkBox->setSizePolicy(QSizePolicy::Preferred,
                                            QSizePolicy::Preferred);
                    moduleCheckBoxes.append(checkBox);
                    moduleLayout->addWidget(checkBox);
                }
                moduleLayout->addStretch();
                for (QCheckBox* checkBox : levelCheckBoxes)
                    checkBox->setChecked(true);
                for (QCheckBox* checkBox : moduleCheckBoxes)
                    checkBox->setChecked(true);

                // Set extra columns on table model and rebuild extra field UI
                sourceModel->setExtraColumns(extraFieldNames);
                rebuildExtraFieldUI(extraFieldNames, extraFieldValues);
            });
    connect(loader, &LogLoader::finished, this,
            [this, loader, thread, filePath]() {
    // 发布构建下，加载结束后恢复视图更新并进行一次性刷新
#ifdef NDEBUG
                logTreeView->setUpdatesEnabled(true);
                logTreeView->viewport()->setUpdatesEnabled(true);
                logTreeView->viewport()->update();
#endif

                progressBar->setVisible(false);
                exportAction->setEnabled(true);
                statusBar()->showMessage(tr("已加载文件：%1，日志条目数：%2")
                                             .arg(filePath)
                                             .arg(sourceModel->rowCount()));
                loader->deleteLater();
                thread->quit();
                thread->wait();
                thread->deleteLater();
            });

    thread->start();
}

void LogViewer::onFilterButtonClicked()
{
    // Clear search state before filter changes to avoid stale proxy row indices
    searchResults.clear();
    currentSearchIndex = -1;
    currentSearchText.clear();
    searchLineEdit->clear();

    QDateTime startTime = startTimeEdit->dateTime();
    QDateTime endTime = endTimeEdit->dateTime();
    QStringList selectedLevels;
    for (QCheckBox* checkBox : levelCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedLevels.append(checkBox->text());
        }
    }
    QStringList selectedModules;
    for (QCheckBox* checkBox : moduleCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedModules.append(checkBox->text());
        }
    }
    if (selectedLevels.isEmpty() || selectedModules.isEmpty()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("请至少选择一个日志等级和模块。"));
        return;
    }

    // Show progress bar
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Indeterminate mode

    if (proxyModel) {
        proxyModel->setTimeRange(startTime, endTime);
        proxyModel->setLevels(selectedLevels);
        proxyModel->setModules(selectedModules);

        // Apply extra field filters
        proxyModel->clearExtraFieldFilters();
        for (auto it = extraFieldCheckBoxes.constBegin();
             it != extraFieldCheckBoxes.constEnd(); ++it) {
            QSet<QString> accepted;
            for (QCheckBox* cb : it.value()) {
                if (cb->isChecked()) {
                    accepted.insert(cb->text());
                }
            }
            if (accepted.size() < it.value().size()) {
                proxyModel->setExtraFieldFilter(it.key(), accepted);
            }
        }
    }

    // Hide progress bar after filtering is complete
    progressBar->setVisible(false);

    statusBar()->showMessage(tr("筛选完成，日志条目数：%1")
                                 .arg(proxyModel ? proxyModel->rowCount() : 0));
}

void LogViewer::rebuildExtraFieldUI(
    const QStringList& fieldNames,
    const QMap<QString, QStringList>& fieldValues)
{
    // Clear existing extra field UI
    QLayoutItem* child;
    while ((child = extraFieldsLayout->takeAt(0)) != nullptr) {
        QWidget* widget = child->widget();
        if (widget)
            widget->deleteLater();
        delete child;
    }
    extraFieldCheckBoxes.clear();

    // Create a GroupBox for each extra field
    for (const QString& fieldName : fieldNames) {
        QGroupBox* groupBox = new QGroupBox(fieldName, this);
        QHBoxLayout* layout = new QHBoxLayout();

        QList<QCheckBox*> checkBoxes;
        QStringList values = fieldValues.value(fieldName);
        for (const QString& value : values) {
            QCheckBox* cb = new QCheckBox(value, this);
            cb->setChecked(true);
            cb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            checkBoxes.append(cb);
            layout->addWidget(cb);
        }
        layout->addStretch();

        groupBox->setLayout(layout);
        extraFieldsLayout->addWidget(groupBox);
        extraFieldCheckBoxes[fieldName] = checkBoxes;
    }
}

void LogViewer::toggleFilterArea()
{
    if (filterWidget->isVisible()) {
        filterWidget->hide();
        toggleFilterAction->setText(tr("显示筛选区域"));
    } else {
        filterWidget->show();
        toggleFilterAction->setText(tr("隐藏筛选区域"));
    }
}

void LogViewer::selectAllModules()
{
    for (QCheckBox* checkBox : moduleCheckBoxes) {
        checkBox->setChecked(true);
    }
}

void LogViewer::deselectAllModules()
{
    for (QCheckBox* checkBox : moduleCheckBoxes) {
        checkBox->setChecked(false);
    }
}

void LogViewer::onSearchTextChanged(const QString& text)
{
    currentSearchText = text;
    searchDebounceTimer->start();
}

void LogViewer::flushSearchDebounce()
{
    if (searchDebounceTimer->isActive()) {
        searchDebounceTimer->stop();
        highlightSearchMatches();
    }
}

void LogViewer::highlightSearchMatches()
{
    logTreeView->setUpdatesEnabled(false);

    // Update delegate highlight text
    if (highlightDelegate)
        highlightDelegate->setHighlightText(currentSearchText);

    searchResults.clear();
    currentSearchIndex = -1;

    if (currentSearchText.isEmpty() || !proxyModel) {
        logTreeView->setUpdatesEnabled(true);
        logTreeView->viewport()->update();
        return;
    }

    // Linear scan on content column for matches (proxy model)
    int rows = proxyModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        QModelIndex contentIdx =
            proxyModel->index(r, LogTableModel::ColumnMessage);
        QString text = proxyModel->data(contentIdx, Qt::DisplayRole).toString();
        if (text.contains(currentSearchText, Qt::CaseInsensitive)) {
            searchResults.append(r);
        }
    }

    logTreeView->setUpdatesEnabled(true);
    logTreeView->viewport()->update();

    if (!searchResults.isEmpty()) {
        currentSearchIndex = 0;
        int row = searchResults[currentSearchIndex];
        QModelIndex index = proxyModel->index(row, 0);
        logTreeView->setCurrentIndex(index);
        logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void LogViewer::expandToItem(QStandardItem* item)
{
    if (!item)
        return;
    QModelIndex index = item->index();
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid()) {
        logTreeView->expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
}

void LogViewer::onSearchPrevious()
{
    flushSearchDebounce();
    if (searchResults.isEmpty())
        return;

    currentSearchIndex--;
    if (currentSearchIndex < 0) {
        currentSearchIndex = searchResults.size() - 1;
    }
    int row = searchResults[currentSearchIndex];
    QModelIndex index = proxyModel->index(row, 0);
    logTreeView->setCurrentIndex(index);
    logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void LogViewer::onSearchNext()
{
    flushSearchDebounce();
    if (searchResults.isEmpty())
        return;

    currentSearchIndex++;
    if (currentSearchIndex >= searchResults.size()) {
        currentSearchIndex = 0;
    }
    int row = searchResults[currentSearchIndex];
    QModelIndex index = proxyModel->index(row, 0);
    logTreeView->setCurrentIndex(index);
    logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void LogViewer::onLogItemDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    QString details;
    int cols = proxyModel->columnCount();
    for (int col = 0; col < cols; ++col) {
        QString header = proxyModel->headerData(col, Qt::Horizontal).toString();
        QString data =
            proxyModel->data(index.sibling(index.row(), col)).toString();
        details += QString("%1: %2\n").arg(header).arg(data);
    }

    QMessageBox::information(this, tr("日志详情"), details);
}

void LogViewer::onTreeItemExpanded(const QModelIndex& index)
{
    Q_UNUSED(index) // Avoid unused parameter warning
    // Avoid expensive full auto-resize on expand; do nothing here
}

void LogViewer::onExportFiltered()
{
    if (sourceModel->rowCount() == 0) {
        QMessageBox::information(this, tr("提示"), tr("没有日志数据可以导出"));
        return;
    }

    // Get current filtered logs
    QList<LogEntry> filteredLogs = getCurrentFilteredLogs();

    if (filteredLogs.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("当前筛选条件下没有日志数据"));
        return;
    }

    // Show export dialog
    ExportDialog dialog(this);
    dialog.setLogCount(filteredLogs.size());

    if (dialog.exec() == QDialog::Accepted) {
        ExportConfig config = dialog.getExportConfig();

        // Create exporter and execute export
        LogExporter* exporter = new LogExporter(this);

        // Connect progress signal
        connect(exporter, &LogExporter::progressChanged,
                [this](int percentage) { progressBar->setValue(percentage); });

        connect(exporter, &LogExporter::formatExported,
                [this](const QString& format, const QString& filePath) {
                    statusBar()->showMessage(
                        tr("已导出 %1 格式到: %2").arg(format).arg(filePath),
                        3000);
                });

        connect(exporter, &LogExporter::exportFinished,
                [this, exporter](bool success, const QString& message) {
                    progressBar->setVisible(false);
                    if (success) {
                        QMessageBox::information(this, tr("导出完成"), message);
                    } else {
                        QMessageBox::warning(this, tr("导出失败"), message);
                    }
                    exporter->deleteLater(); // Clean up exporter object
                });

        // Show progress bar
        progressBar->setVisible(true);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);

        // TODO: Export runs on the main thread and will block the UI for large
        // datasets. This should be moved to a worker thread (QThread) in the
        // future so the UI remains responsive during export.

        // Execute export
        if (config.formats.size() > 1) {
            exporter->exportMultipleFormats(filteredLogs, config);
        } else {
            exporter->exportLogs(filteredLogs, config);
        }
    }
}

void LogViewer::onExportSearchResults()
{
    flushSearchDebounce();
    if (currentSearchText.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先输入搜索内容"));
        return;
    }

    if (searchResults.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("没有找到匹配的搜索结果"));
        return;
    }

    // Get search matched logs
    QList<LogEntry> matchedLogs = getSearchMatchedLogs();

    if (matchedLogs.isEmpty()) {
        QMessageBox::information(this, tr("提示"),
                                 tr("没有匹配的日志数据可以导出"));
        return;
    }

    // Show export dialog
    ExportDialog dialog(this);
    dialog.setWindowTitle(tr("导出搜索结果"));
    dialog.setLogCount(matchedLogs.size());

    if (dialog.exec() == QDialog::Accepted) {
        ExportConfig config = dialog.getExportConfig();

        // Create exporter and execute export
        LogExporter* exporter = new LogExporter(this);

        // Connect progress signal
        connect(exporter, &LogExporter::progressChanged,
                [this](int percentage) { progressBar->setValue(percentage); });

        connect(exporter, &LogExporter::formatExported,
                [this](const QString& format, const QString& filePath) {
                    statusBar()->showMessage(
                        tr("已导出 %1 格式到: %2").arg(format).arg(filePath),
                        3000);
                });

        connect(exporter, &LogExporter::exportFinished,
                [this, exporter](bool success, const QString& message) {
                    progressBar->setVisible(false);
                    if (success) {
                        QMessageBox::information(this, tr("导出完成"), message);
                    } else {
                        QMessageBox::warning(this, tr("导出失败"), message);
                    }
                    exporter->deleteLater(); // Clean up exporter object
                });

        // Show progress bar
        progressBar->setVisible(true);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);

        // TODO: Export runs on the main thread and will block the UI for large
        // datasets. This should be moved to a worker thread (QThread) in the
        // future so the UI remains responsive during export.

        // Execute export
        if (config.formats.size() > 1) {
            exporter->exportMultipleFormats(matchedLogs, config);
        } else {
            exporter->exportLogs(matchedLogs, config);
        }
    }
}

void LogViewer::onFormatTemplateAction()
{
    FormatTemplateDialog dialog(this);
    dialog.setTemplate(AppSettings::instance().getLogFormatTemplate());
    if (dialog.exec() == QDialog::Accepted) {
        QString newTemplate = dialog.getTemplate();
        AppSettings::instance().setLogFormatTemplate(newTemplate);
        // If a file is already loaded, offer to reload with new format
        if (!currentFilePath.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Reload"),
                tr("Format changed. Reload the current file with the new format?"));
            if (reply == QMessageBox::Yes) {
                loadLogFile(currentFilePath);
            }
        }
    }
}

QList<LogEntry> LogViewer::getCurrentFilteredLogs() const
{
    if (!proxyModel || !sourceModel) {
        return QList<LogEntry>();
    }

    QList<LogEntry> filteredLogs;

    for (int i = 0; i < proxyModel->rowCount(); ++i) {
        QModelIndex srcIndex = proxyModel->mapToSource(proxyModel->index(i, 0));
        int srcRow = srcIndex.row();
        if (srcRow >= 0 && srcRow < sourceModel->size())
            filteredLogs.append(sourceModel->at(srcRow));
    }

    return filteredLogs;
}

QList<LogEntry> LogViewer::getSearchMatchedLogs() const
{
    if (!proxyModel || !sourceModel || searchResults.isEmpty()) {
        return QList<LogEntry>();
    }

    QList<LogEntry> matchedLogs;

    for (int proxyRow : searchResults) {
        if (proxyRow >= 0 && proxyRow < proxyModel->rowCount()) {
            QModelIndex srcIndex =
                proxyModel->mapToSource(proxyModel->index(proxyRow, 0));
            int srcRow = srcIndex.row();
            if (srcRow >= 0 && srcRow < sourceModel->size()) {
                matchedLogs.append(sourceModel->at(srcRow));
            }
        }
    }

    return matchedLogs;
}

void LogViewer::onLanguageChanged(int index)
{
    // Get selected language
    if (index >= 0 && index < languageComboBox->count()) {
        int languageValue = languageComboBox->itemData(index).toInt();
        LanguageManager::Language language =
            static_cast<LanguageManager::Language>(languageValue);

        // Instant language switch, no restart needed
        LanguageManager::instance().setLanguage(language);
    }
}

void LogViewer::onLanguageManagerChanged(LanguageManager::Language language)
{
    // Update dropdown current selection
    int newIndex = languageComboBox->findData(static_cast<int>(language));
    if (newIndex >= 0 && newIndex != languageComboBox->currentIndex()) {
        languageComboBox->blockSignals(true);
        languageComboBox->setCurrentIndex(newIndex);
        languageComboBox->blockSignals(false);
    }

    // Re-translate UI
    retranslateUI();
}

void LogViewer::retranslateUI()
{
#ifdef LOG_DEBUG_ENABLED
    qDebug() << "=== Starting UI Retranslation ===";

    // Test translation to work
    QString testTranslation = tr("日志查看器");
    qDebug() << "Test translation result:" << testTranslation;
#endif

    // Re-set window title
    setWindowTitle(tr("日志查看器"));
#ifdef LOG_DEBUG_ENABLED
    qDebug() << "Window title updated to:" << windowTitle();
#endif

    // Re-set menu item text
    if (fileMenu)
        fileMenu->setTitle(tr("文件"));

    // Re-set action text
    if (openAction) {
        QString oldText = openAction->text();
        openAction->setText(tr("打开日志文件"));
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Open action text changed from" << oldText << "to"
                 << openAction->text();
#endif
    }
    if (exportAction)
        exportAction->setText(tr("导出筛选结果"));
    if (filterAction)
        filterAction->setText(tr("筛选"));
    if (toggleFilterAction)
        toggleFilterAction->setText(tr("显示/隐藏筛选区域"));
    if (formatTemplateAction)
        formatTemplateAction->setText(tr("日志格式"));

    // Re-set tool bar title
    QList<QToolBar*> toolBars = findChildren<QToolBar*>();
    for (QToolBar* toolBar : toolBars) {
        toolBar->setWindowTitle(tr("工具栏"));
    }

    // Re-set group box title
    if (timeGroupBox) {
        QString oldTitle = timeGroupBox->title();
        timeGroupBox->setTitle(tr("时间范围"));
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Time group title changed from" << oldTitle << "to"
                 << timeGroupBox->title();
#endif
    }
    if (levelGroupBox) {
        QString oldTitle = levelGroupBox->title();
        levelGroupBox->setTitle(tr("日志等级"));
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Level group title changed from" << oldTitle << "to"
                 << levelGroupBox->title();
#endif
    }
    if (moduleGroupBox) {
        QString oldTitle = moduleGroupBox->title();
        moduleGroupBox->setTitle(tr("模块选择"));
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Module group title changed from" << oldTitle << "to"
                 << moduleGroupBox->title();
#endif
    }

    // Re-set button text
    if (selectAllModulesButton)
        selectAllModulesButton->setText(tr("全选"));
    if (deselectAllModulesButton)
        deselectAllModulesButton->setText(tr("全不选"));
    if (searchPreviousButton)
        searchPreviousButton->setText(tr("上一条"));
    if (searchNextButton)
        searchNextButton->setText(tr("下一条"));
    if (exportSearchResultsButton)
        exportSearchResultsButton->setText(tr("导出搜索结果"));

    // Re-set search line edit placeholder text
    if (searchLineEdit)
        searchLineEdit->setPlaceholderText(tr("搜索..."));

    // Re-set label text directly via member pointers
    if (startTimeLabel)
        startTimeLabel->setText(tr("开始时间:"));
    if (endTimeLabel)
        endTimeLabel->setText(tr("结束时间:"));
    if (encodingLabel)
        encodingLabel->setText(tr("文件编码:"));
    if (languageLabel)
        languageLabel->setText(tr("语言:"));

    // Re-set table headers (via view header, since we use custom model)
    if (logTreeView && logTreeView->header()) {
        // Nothing needed; model provides translated headers
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Table headers updated via model";
#endif
    }

    // Re-set status bar information
    if (statusBar()) {
        statusBar()->showMessage(tr("就绪"));
    }

    // Force UI update
    update();
    repaint(); // Force redraw

    // Update GitHub link text and tooltip
    if (githubLinkLabel) {
        githubLinkLabel->setToolTip(tr("点击访问GitHub项目页面并给我们点赞"));
    }

#ifdef LOG_DEBUG_ENABLED
    qDebug() << "=== UI Retranslation Completed ===";
#endif
}

bool LogViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == githubLinkLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onGitHubLinkClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void LogViewer::onGitHubLinkClicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/GeziP/LogReader"));
}
