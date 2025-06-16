#include "logviewer.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QProgressBar>
#include <QGroupBox>
#include <QSet>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include <QStyleFactory>
#include <QFont>
#include <QStatusBar>
#include <QScrollArea>
#include <QSizePolicy>
#include <QLineEdit>
#include <QBrush>
#include <QSplitter>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QFontMetrics>

LogViewer::LogViewer(QWidget *parent)
    : QMainWindow(parent),
      logModel(nullptr),
      currentSearchIndex(-1)
{
    setupUI();
}

void LogViewer::setupUI()
{
    // 设置窗口标题和图标
    setWindowTitle(tr("日志查看器"));
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    // 设置应用程序样式
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // 设置全局字体
    QFont defaultFont = this->font();
    defaultFont.setPointSize(10);
    this->setFont(defaultFont);

    // 初始化日志字体
    logFont = defaultFont;

    // 设置样式表
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
        selection-background-color: #3399FF; /* 选中行的背景色 */
        selection-color: white; /* 选中行的文字颜色 */
    }

    QTreeView::item {
        background-color: white;
        color: black;
    }

    QTreeView::item:selected {
        background-color: #3399FF; /* 选中行的背景色 */
        color: white; /* 选中行的文字颜色 */
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

    qApp->setStyleSheet(qss);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 筛选区域容器
    filterWidget = new QWidget(this);

    // 时间范围选择
    QLabel *startLabel = new QLabel(tr("开始时间:"), this);
    startTimeEdit = new QDateTimeEdit(this);
    startTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    QLabel *endLabel = new QLabel(tr("结束时间:"), this);
    endTimeEdit = new QDateTimeEdit(this);
    endTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(startLabel);
    timeLayout->addWidget(startTimeEdit);
    timeLayout->addWidget(endLabel);
    timeLayout->addWidget(endTimeEdit);
    timeLayout->addStretch();

    timeGroupBox = new QGroupBox(tr("时间范围"), this);
    timeGroupBox->setLayout(timeLayout);

    // 日志等级选择
    levelGroupBox = new QGroupBox(tr("日志等级"), this);
    QHBoxLayout *levelLayout = new QHBoxLayout();
    levelGroupBox->setLayout(levelLayout);

    QStringList levels = {"DEBUG", "ERROR", "INFO", "WARN"};
    levels.sort(Qt::CaseInsensitive); // 按字典序排序

    for (const QString &level : levels) {
        QCheckBox *checkBox = new QCheckBox(level, this);
        checkBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        levelCheckBoxes.append(checkBox);
        levelLayout->addWidget(checkBox);
    }

    // 添加弹性空间，左对齐
    levelLayout->addStretch();


    // 模块选择
    moduleLayout = new QHBoxLayout();

    // 添加“全选”和“全不选”按钮
    selectAllModulesButton = new QPushButton(tr("全选"), this);
    deselectAllModulesButton = new QPushButton(tr("全不选"), this);
    connect(selectAllModulesButton, &QPushButton::clicked, this, &LogViewer::selectAllModules);
    connect(deselectAllModulesButton, &QPushButton::clicked, this, &LogViewer::deselectAllModules);

    QHBoxLayout *moduleButtonLayout = new QHBoxLayout();
    moduleButtonLayout->addWidget(selectAllModulesButton);
    moduleButtonLayout->addWidget(deselectAllModulesButton);
    moduleButtonLayout->addStretch();

    // 添加滚动区域以支持模块选择过多的情况
    QScrollArea *moduleScrollArea = new QScrollArea(this);
    QWidget *moduleContainer = new QWidget(this);
    moduleLayout->setContentsMargins(5, 5, 5, 5);
    moduleContainer->setLayout(moduleLayout);
    moduleScrollArea->setWidgetResizable(true);
    moduleScrollArea->setWidget(moduleContainer);

    // 将滚动方向设置为水平滚动
    moduleScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    moduleScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置固定高度，避免出现垂直滚动条
    moduleScrollArea->setFixedHeight(60); // 根据需要调整高度

    moduleGroupBox = new QGroupBox(tr("模块选择"), this);
    QVBoxLayout *moduleGroupLayout = new QVBoxLayout();
    moduleGroupLayout->addLayout(moduleButtonLayout); // 添加按钮布局
    moduleGroupLayout->addWidget(moduleScrollArea);
    moduleGroupBox->setLayout(moduleGroupLayout);


    // 编码选择
    QLabel *encodingLabel = new QLabel(tr("文件编码:"), this);
    encodingComboBox = new QComboBox(this);
    encodingComboBox->addItems({"UTF-8", "GB18030", "GB2312"});
    encodingComboBox->setCurrentText("UTF-8");

    QHBoxLayout *encodingLayout = new QHBoxLayout();
    encodingLayout->addWidget(encodingLabel);
    encodingLayout->addWidget(encodingComboBox);
    encodingLayout->addStretch();

    // 筛选按钮
    QPushButton *filterButton = new QPushButton(tr("筛选"), this);
    connect(filterButton, &QPushButton::clicked, this, &LogViewer::onFilterButtonClicked);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addStretch();
    filterLayout->addWidget(filterButton);

    // 将筛选区域组合在一起
    QVBoxLayout *filterAreaLayout = new QVBoxLayout();
    filterAreaLayout->addWidget(timeGroupBox);
    filterAreaLayout->addWidget(levelGroupBox);
    filterAreaLayout->addWidget(moduleGroupBox);
    filterAreaLayout->addLayout(encodingLayout);
    filterAreaLayout->addLayout(filterLayout);
    filterWidget->setLayout(filterAreaLayout);

    // 日志树视图
    logTreeView = new QTreeView(this);
    logTreeView->setAlternatingRowColors(true);
    logTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTreeView->setUniformRowHeights(false);
    logTreeView->setHeaderHidden(false);
    logTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    logTreeView->setFont(logFont); // 设置初始字体

    // 添加以下两行代码
    logTreeView->setItemsExpandable(true);
    logTreeView->setRootIsDecorated(true);

    // 设置水平滚动条
    logTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 连接双击信号
    connect(logTreeView, &QTreeView::doubleClicked, this, &LogViewer::onLogItemDoubleClicked);
    // 在 setupUI 或构造函数中连接展开信号
    connect(logTreeView, &QTreeView::expanded, this, &LogViewer::onTreeItemExpanded);

    // 搜索框和按钮
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("搜索..."));
    connect(searchLineEdit, &QLineEdit::textChanged, this, &LogViewer::onSearchTextChanged);

    searchPreviousButton = new QPushButton(tr("上一条"), this);
    searchNextButton = new QPushButton(tr("下一条"), this);
    connect(searchPreviousButton, &QPushButton::clicked, this, &LogViewer::onSearchPrevious);
    connect(searchNextButton, &QPushButton::clicked, this, &LogViewer::onSearchNext);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchPreviousButton);
    searchLayout->addWidget(searchNextButton);
    searchLayout->addStretch();

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 使用 QSplitter 分隔筛选区域和日志视图
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(filterWidget);
    mainSplitter->addWidget(logTreeView);
    mainSplitter->setStretchFactor(1, 1); // 让日志视图占据更多空间

    mainLayout->addWidget(mainSplitter);
    mainLayout->addLayout(searchLayout);

    centralWidget->setLayout(mainLayout);

    // 菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = menuBar->addMenu(tr("文件"));
    openAction = fileMenu->addAction(QIcon(":/icons/open.png"), tr("打开日志文件"));
    connect(openAction, &QAction::triggered, this, &LogViewer::openLogFile);
    setMenuBar(menuBar);

    // 工具栏
    QToolBar *toolBar = addToolBar(tr("工具栏"));
    toolBar->addAction(openAction);
    filterAction = toolBar->addAction(QIcon(":/icons/filter.png"), tr("筛选"));
    connect(filterAction, &QAction::triggered, this, &LogViewer::onFilterButtonClicked);

    // 显示/隐藏筛选区域的按钮
    toggleFilterAction = toolBar->addAction(QIcon(":/icons/toggle.png"), tr("显示/隐藏筛选区域"));
    connect(toggleFilterAction, &QAction::triggered, this, &LogViewer::toggleFilterArea);

    // 状态栏
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    statusBar()->addPermanentWidget(progressBar);
    statusBar()->showMessage(tr("就绪"));

}

void LogViewer::openLogFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("打开日志文件"), "", tr("Log Files (*.log *.txt);;All Files (*)"));
    if (!filePath.isEmpty()) {
        loadLogFile(filePath);
    }
}

void LogViewer::loadLogFile(const QString& filePath)
{
    currentFilePath = filePath;
    QString encoding = encodingComboBox->currentText();

    // 显示进度条
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // 不确定模式

    allLogs = parseLogFile(filePath, encoding);

    // 加载完成后隐藏进度条
    progressBar->setVisible(false);

    if (allLogs.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("日志文件为空或无法解析。"));
        return;
    }

    // 初始化开始和结束时间
    QDateTime minTime = allLogs.first().timestamp;
    QDateTime maxTime = allLogs.first().timestamp;
    QSet<QString> moduleSet;
    QSet<QString> levelSet;
    for (const LogEntry &entry : allLogs) {
        if (entry.timestamp.isValid()) {
            if (entry.timestamp < minTime)
                minTime = entry.timestamp;
            if (entry.timestamp > maxTime)
                maxTime = entry.timestamp;
        }
        moduleSet.insert(entry.module);
        levelSet.insert(entry.level);
    }

    startTimeEdit->setDateTime(minTime);
    endTimeEdit->setDateTime(maxTime);

    // 更新模块列表
    allModules = moduleSet.values();
    allModules.sort(Qt::CaseInsensitive); // 按字典序排序
    allLevels = levelSet.values();
    allLevels.sort(Qt::CaseInsensitive); // 按字典序排序

    // 清除现有的模块选择框
    QLayoutItem *child;
    while ((child = moduleLayout->takeAt(0)) != nullptr) {
        QWidget *widget = child->widget();
        if (widget) {
            widget->deleteLater(); // 安全地删除控件
        }
        delete child; // 删除布局项
    }
    moduleCheckBoxes.clear();

    // 添加新的模块选择框
    for (const QString &module : allModules) {
        QCheckBox *checkBox = new QCheckBox(module, this);
        checkBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        moduleCheckBoxes.append(checkBox);
        moduleLayout->addWidget(checkBox);
    }

    // 可选：添加弹性空间，左对齐
    moduleLayout->addStretch();

    // 默认选择所有等级和模块
    for (QCheckBox *checkBox : levelCheckBoxes) {
        checkBox->setChecked(true);
    }
    for (QCheckBox *checkBox : moduleCheckBoxes) {
        checkBox->setChecked(true);
    }

    // 显示所有日志
    displayLogs(allLogs);

    statusBar()->showMessage(tr("已加载文件：%1，日志条目数：%2").arg(filePath).arg(allLogs.size()));
}

QList<LogEntry> LogViewer::parseLogFile(const QString& filePath, const QString& encoding)
{
    QList<LogEntry> logEntries;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开日志文件。"));
        return logEntries;
    }

    QTextStream in(&file);
    // 处理中文编码
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
    if (!codec) {
        QMessageBox::warning(this, tr("错误"), tr("不支持的编码格式：%1").arg(encoding));
        return logEntries;
    }
    in.setCodec(codec);
#else
    in.setEncoding(QStringConverter::Utf8);
#endif

    QRegularExpression regex(R"(\[(.*?)\] \[(.*?)\] \[(.*?)\] : (.*))");
    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            LogEntry entry;
            entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd HH:mm:ss.zzz");
            if (!entry.timestamp.isValid()) {
                // 尝试其他格式
                entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd HH:mm:ss");
            }
            entry.level = match.captured(2).trimmed();
            entry.module = match.captured(3).trimmed();
            entry.content = match.captured(4);
            logEntries.append(entry);
        }
    }
    return logEntries;
}

void LogViewer::onFilterButtonClicked()
{
    QDateTime startTime = startTimeEdit->dateTime();
    QDateTime endTime = endTimeEdit->dateTime();
    QStringList selectedLevels;
    for (QCheckBox *checkBox : levelCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedLevels.append(checkBox->text());
        }
    }
    QStringList selectedModules;
    for (QCheckBox *checkBox : moduleCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedModules.append(checkBox->text());
        }
    }
    if (selectedLevels.isEmpty() || selectedModules.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请至少选择一个日志等级和模块。"));
        return;
    }

    // 显示进度条
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // 不确定模式

    QList<LogEntry> filteredLogs = filterLogs(allLogs, startTime, endTime, selectedLevels, selectedModules);

    // 筛选完成后隐藏进度条
    progressBar->setVisible(false);

    displayLogs(filteredLogs);

    statusBar()->showMessage(tr("筛选完成，日志条目数：%1").arg(filteredLogs.size()));
}

QList<LogEntry> LogViewer::filterLogs(const QList<LogEntry>& logs,
                                      const QDateTime& startTime,
                                      const QDateTime& endTime,
                                      const QStringList& levels,
                                      const QStringList& modules)
{
    QList<LogEntry> filteredLogs;
    for (const auto& entry : logs) {
        if (!entry.timestamp.isValid()) continue;
        if (entry.timestamp >= startTime && entry.timestamp <= endTime &&
            levels.contains(entry.level) &&
            modules.contains(entry.module)) {
            filteredLogs.append(entry);
        }
    }
    return filteredLogs;
}

void LogViewer::displayLogs(const QList<LogEntry>& logs)
{
    if (logModel) {
        delete logModel;
    }
    logModel = new QStandardItemModel(this);
    QStringList headers = { tr("行号"), tr("时间"), tr("等级"), tr("模块"), tr("内容") };
    logModel->setHorizontalHeaderLabels(headers);

    int totalLogs = allLogs.size();
    int filterIndex = 0;
    int i = 0;
    int lineNumber = 1; // 行号从1开始

    while (i < totalLogs) {
        LogEntry currentEntry = allLogs[i];
        bool matchesFilter = false;
        if (filterIndex < logs.size() && logs[filterIndex].timestamp == currentEntry.timestamp) {
            matchesFilter = true;
            filterIndex++;
        }

        if (matchesFilter) {
            // 匹配筛选条件，直接添加为顶层项
            QList<QStandardItem*> rowItems;
            QStandardItem *lineNumberItem = new QStandardItem(QString::number(lineNumber++));
            lineNumberItem->setTextAlignment(Qt::AlignCenter);  // 行号居中显示
            QStandardItem *timestampItem = new QStandardItem(currentEntry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"));
            QStandardItem *levelItem = new QStandardItem(currentEntry.level);
            QStandardItem *moduleItem = new QStandardItem(currentEntry.module);
            QStandardItem *contentItem = new QStandardItem(currentEntry.content);

            rowItems << lineNumberItem << timestampItem << levelItem << moduleItem << contentItem;
            logModel->appendRow(rowItems);
            i++;
        } else {
            // 不匹配筛选条件，收集连续的非匹配日志
            int start = i;
            while (i < totalLogs) {
                LogEntry entry = allLogs[i];
                bool matchesFilterInner = false;
                if (filterIndex < logs.size() && logs[filterIndex].timestamp == entry.timestamp) {
                    matchesFilterInner = true;
                }
                if (matchesFilterInner) {
                    break;
                }
                i++;
            }
            int end = i - 1;

            // 创建折叠项
            int count = end - start + 1;
            QList<QStandardItem*> foldRow;
            foldRow << new QStandardItem(""); // 行号为空
            foldRow << new QStandardItem(""); // 时间为空
            foldRow << new QStandardItem(""); // 等级为空
            foldRow << new QStandardItem(""); // 模块为空
            QStandardItem *foldItemContent = new QStandardItem(tr("隐藏的日志 (%1 条)").arg(count));
            foldItemContent->setForeground(QBrush(Qt::gray));
            foldItemContent->setEditable(false);
            foldRow << foldItemContent; // 内容列显示折叠信息

            logModel->appendRow(foldRow);

            // 获取刚刚添加的父项（折叠项）
            QStandardItem *foldItem = logModel->item(logModel->rowCount() - 1);

            // 将不匹配的日志作为子项添加到折叠项下
            for (int j = start; j <= end; ++j) {
                const auto& entry = allLogs[j];
                QList<QStandardItem*> childRow;
                QStandardItem *childLineNumberItem = new QStandardItem(QString::number(lineNumber++));
                childLineNumberItem->setTextAlignment(Qt::AlignCenter);  // 子项行号也居中
                QStandardItem *childTimestampItem = new QStandardItem(entry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"));
                QStandardItem *childLevelItem = new QStandardItem(entry.level);
                QStandardItem *childModuleItem = new QStandardItem(entry.module);
                QStandardItem *childContentItem = new QStandardItem(entry.content);

                childRow << childLineNumberItem << childTimestampItem << childLevelItem << childModuleItem << childContentItem;
                foldItem->appendRow(childRow);
            }

            // 默认折叠
            QModelIndex foldIndex = logModel->index(logModel->rowCount() - 1, 0);
            logTreeView->setExpanded(foldIndex, false);
        }
    }

    // 设置模型
    logTreeView->setModel(logModel);

    // 禁用最后一列自动拉伸
    logTreeView->header()->setStretchLastSection(false);

    // 设置所有列的 resizeMode 为 Interactive，允许用户调整
    for (int i = 0; i < logModel->columnCount(); ++i) {
        logTreeView->header()->setSectionResizeMode(i, QHeaderView::Interactive);
    }

    // 自动调整列宽度以适应内容
    for (int i = 0; i < logModel->columnCount(); ++i) {
        logTreeView->resizeColumnToContents(i);
    }

    // 设置列的最小宽度，防止过窄
    logTreeView->header()->setMinimumSectionSize(50);

    // 设置行号列宽度
    logTreeView->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    logTreeView->header()->resizeSection(0, 80); // 增加行号列宽度为 80 像素
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
    for (QCheckBox *checkBox : moduleCheckBoxes) {
        checkBox->setChecked(true);
    }
}

void LogViewer::deselectAllModules()
{
    for (QCheckBox *checkBox : moduleCheckBoxes) {
        checkBox->setChecked(false);
    }
}

void LogViewer::onSearchTextChanged(const QString &text)
{
    currentSearchText = text;
    highlightSearchMatches();
}

void LogViewer::highlightSearchMatches()
{
    // 禁用界面更新
    logTreeView->setUpdatesEnabled(false);

    clearSearchHighlights();
    searchResults.clear();
    currentSearchIndex = -1;

    if (currentSearchText.isEmpty() || !logModel) {
        // 启用界面更新
        logTreeView->setUpdatesEnabled(true);
        return;
    }

    // 从根节点开始搜索
    for (int i = 0; i < logModel->rowCount(); ++i) {
        QStandardItem *item = logModel->item(i);
        searchInItem(item);
    }

    // 启用界面更新
    logTreeView->setUpdatesEnabled(true);

    // 跳转到第一个匹配项
    if (!searchResults.isEmpty()) {
        currentSearchIndex = 0;
        QStandardItem *item = searchResults[currentSearchIndex];
        expandToItem(item);
        QModelIndex index = item->index();
        logTreeView->setCurrentIndex(index);
        logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void LogViewer::expandToItem(QStandardItem *item)
{
    if (!item) return;
    QModelIndex index = item->index();
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid()) {
        logTreeView->expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
}

void LogViewer::searchInItem(QStandardItem *item)
{
    if (!item) return;

    bool matched = false;

    // 检查内容列（索引为4）
    QStandardItem *contentItem = item->child(item->row(), 4);
    if (!contentItem) {
        contentItem = item->model()->itemFromIndex(item->index().sibling(item->row(), 4));
    }
    if (contentItem && contentItem->text().contains(currentSearchText, Qt::CaseInsensitive)) {
        matched = true;
        contentItem->setBackground(QBrush(Qt::yellow));
    }

    // 如果匹配，添加到结果列表
    if (matched) {
        searchResults.append(item);
    }

    // 递归搜索子项
    int childCount = item->rowCount();
    for (int i = 0; i < childCount; ++i) {
        QStandardItem *childItem = item->child(i);
        searchInItem(childItem);
    }
}

void LogViewer::clearHighlightsInItem(QStandardItem *item)
{
    if (!item) return;

    // 清除当前项的背景色
    for (int col = 0; col < item->columnCount(); ++col) {
        QStandardItem *cellItem = item->child(item->row(), col);
        if (!cellItem) {
            cellItem = item->model()->itemFromIndex(item->index().sibling(item->row(), col));
        }
        if (cellItem) {
            cellItem->setBackground(QBrush(Qt::NoBrush));
        }
    }

    // 递归处理子项
    int childCount = item->rowCount();
    for (int i = 0; i < childCount; ++i) {
        QStandardItem *childItem = item->child(i);
        clearHighlightsInItem(childItem);
    }
}

void LogViewer::clearSearchHighlights()
{
    if (!logModel) return;

    // 递归清除所有项的高亮
    for (int i = 0; i < logModel->rowCount(); ++i) {
        QStandardItem *item = logModel->item(i);
        clearHighlightsInItem(item);
    }
}

void LogViewer::onSearchPrevious()
{
    if (searchResults.isEmpty()) return;

    currentSearchIndex--;
    if (currentSearchIndex < 0) {
        currentSearchIndex = searchResults.size() - 1;
    }
    QStandardItem *item = searchResults[currentSearchIndex];
    expandToItem(item);
    QModelIndex index = item->index();
    logTreeView->setCurrentIndex(index);
    logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void LogViewer::onSearchNext()
{
    if (searchResults.isEmpty()) return;

    currentSearchIndex++;
    if (currentSearchIndex >= searchResults.size()) {
        currentSearchIndex = 0;
    }
    QStandardItem *item = searchResults[currentSearchIndex];
    expandToItem(item);
    QModelIndex index = item->index();
    logTreeView->setCurrentIndex(index);
    logTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void LogViewer::onLogItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QModelIndex sourceIndex = index;
    // 如果是折叠项，获取其子项
    if (logModel->hasChildren(index)) {
        sourceIndex = logModel->index(0, 0, index);
    }

    QString details;
    for (int col = 0; col < logModel->columnCount(); ++col) {
        QString header = logModel->headerData(col, Qt::Horizontal).toString();
        QString data = logModel->data(sourceIndex.sibling(sourceIndex.row(), col)).toString();
        details += QString("%1: %2\n").arg(header).arg(data);
    }

    QMessageBox::information(this, tr("日志详情"), details);
}

void LogViewer::onTreeItemExpanded(const QModelIndex &index)
{
    // 调整所有列的宽度以适应内容
    for (int i = 0; i < logModel->columnCount(); ++i) {
        logTreeView->resizeColumnToContents(i);
    }
}
