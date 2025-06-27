#include "exportdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent), logCount(0)
{
    setWindowTitle(tr("导出筛选结果"));
    setModal(true);
    resize(450, 400);
    
    setupUI();
    setupConnections();
    
    // 从配置中恢复上次的设置
    loadSettings();
    
    onFormatChanged();
    updatePreview();
}

ExportConfig ExportDialog::getExportConfig() const
{
    ExportConfig config;
    
    // 获取选中的格式列表
    if (txtCheckBox->isChecked()) {
        config.formats.append(ExportConfig::TXT);
    }
    if (csvCheckBox->isChecked()) {
        config.formats.append(ExportConfig::CSV);
    }
    if (jsonCheckBox->isChecked()) {
        config.formats.append(ExportConfig::JSON);
    }
    
    // 获取字段选择
    config.includeTimestamp = timestampCheckBox->isChecked();
    config.includeLevel = levelCheckBox->isChecked();
    config.includeModule = moduleCheckBox->isChecked();
    config.includeContent = contentCheckBox->isChecked();
    
    // 设置导出目录和基础文件名
    config.exportDir = filePathLineEdit->text(); // 直接使用选择的目录
    config.baseFileName = getDefaultFileName();  // 使用自动生成的文件名
    
    return config;
}

void ExportDialog::setLogCount(int count)
{
    logCount = count;
    logCountLabel->setText(tr("将导出 %1 条日志记录").arg(count));
}

void ExportDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 日志数量标签
    logCountLabel = new QLabel(this);
    logCountLabel->setStyleSheet("color: #666; font-weight: bold;");
    mainLayout->addWidget(logCountLabel);
    
    // 格式选择组（支持多选）
    formatGroupBox = new QGroupBox(tr("导出格式（可多选）"), this);
    QHBoxLayout *formatLayout = new QHBoxLayout(formatGroupBox);
    
    txtCheckBox = new QCheckBox(tr("文本文件 (*.txt)"), this);
    csvCheckBox = new QCheckBox(tr("CSV文件 (*.csv)"), this);
    jsonCheckBox = new QCheckBox(tr("JSON文件 (*.json)"), this);
    
    formatLayout->addWidget(txtCheckBox);
    formatLayout->addWidget(csvCheckBox);
    formatLayout->addWidget(jsonCheckBox);
    formatLayout->addStretch();
    
    mainLayout->addWidget(formatGroupBox);
    
    // 字段选择组
    fieldsGroupBox = new QGroupBox(tr("包含字段"), this);
    QVBoxLayout *fieldsLayout = new QVBoxLayout(fieldsGroupBox);
    
    timestampCheckBox = new QCheckBox(tr("时间戳"), this);
    levelCheckBox = new QCheckBox(tr("日志等级"), this);
    moduleCheckBox = new QCheckBox(tr("模块名称"), this);
    contentCheckBox = new QCheckBox(tr("日志内容"), this);
    
    QHBoxLayout *fieldsRow1 = new QHBoxLayout();
    fieldsRow1->addWidget(timestampCheckBox);
    fieldsRow1->addWidget(levelCheckBox);
    fieldsRow1->addStretch();
    
    QHBoxLayout *fieldsRow2 = new QHBoxLayout();
    fieldsRow2->addWidget(moduleCheckBox);
    fieldsRow2->addWidget(contentCheckBox);
    fieldsRow2->addStretch();
    
    fieldsLayout->addLayout(fieldsRow1);
    fieldsLayout->addLayout(fieldsRow2);
    
    mainLayout->addWidget(fieldsGroupBox);
    
    // 文件路径选择组
    fileGroupBox = new QGroupBox(tr("保存位置"), this);
    QVBoxLayout *fileGroupLayout = new QVBoxLayout(fileGroupBox);
    
    // 添加说明标签
    exportHintLabel = new QLabel(this);
    exportHintLabel->setStyleSheet("color: #666; font-size: 11px;");
    exportHintLabel->setWordWrap(true);
    
    QHBoxLayout *fileLayout = new QHBoxLayout();
    filePathLineEdit = new QLineEdit(this);
    filePathLineEdit->setPlaceholderText(tr("选择导出文件夹，文件名自动生成..."));
    browseButton = new QPushButton(tr("选择文件夹..."), this);
    
    fileLayout->addWidget(filePathLineEdit);
    fileLayout->addWidget(browseButton);
    
    fileGroupLayout->addWidget(exportHintLabel);
    fileGroupLayout->addLayout(fileLayout);
    
    mainLayout->addWidget(fileGroupBox);
    
    // 预览标签
    QLabel *previewTitleLabel = new QLabel(tr("预览格式:"), this);
    previewTitleLabel->setStyleSheet("font-weight: bold;");
    previewLabel = new QLabel(this);
    previewLabel->setStyleSheet("background-color: #f5f5f5; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-family: monospace;");
    previewLabel->setWordWrap(true);
    previewLabel->setMinimumHeight(60);
    
    mainLayout->addWidget(previewTitleLabel);
    mainLayout->addWidget(previewLabel);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    okButton = new QPushButton(tr("导出"), this);
    cancelButton = new QPushButton(tr("取消"), this);
    
    okButton->setDefault(true);
    okButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px 16px; border-radius: 4px; font-weight: bold;");
    cancelButton->setStyleSheet("background-color: #f44336; color: white; padding: 8px 16px; border-radius: 4px;");
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ExportDialog::setupConnections()
{
    connect(txtCheckBox, &QCheckBox::toggled, this, &ExportDialog::onFormatChanged);
    connect(csvCheckBox, &QCheckBox::toggled, this, &ExportDialog::onFormatChanged);
    connect(jsonCheckBox, &QCheckBox::toggled, this, &ExportDialog::onFormatChanged);
    
    connect(timestampCheckBox, &QCheckBox::toggled, this, &ExportDialog::updatePreview);
    connect(levelCheckBox, &QCheckBox::toggled, this, &ExportDialog::updatePreview);
    connect(moduleCheckBox, &QCheckBox::toggled, this, &ExportDialog::updatePreview);
    connect(contentCheckBox, &QCheckBox::toggled, this, &ExportDialog::updatePreview);
    
    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::onBrowseFileClicked);
    connect(okButton, &QPushButton::clicked, this, &ExportDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ExportDialog::onBrowseFileClicked()
{
    // 检查选中的格式数量
    QStringList selectedFormats = getSelectedFormats();
    
    if (selectedFormats.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择至少一种导出格式！"));
        return;
    }
    
    // 使用记忆的导出目录作为默认路径
    QString defaultPath = AppSettings::instance().getRecentExportDir();
    
    QString directoryPath = QFileDialog::getExistingDirectory(
        this,
        tr("选择导出文件夹"),
        defaultPath
    );
    
    if (!directoryPath.isEmpty()) {
        filePathLineEdit->setText(directoryPath);
        // 保存新的导出目录
        AppSettings::instance().setRecentExportDir(directoryPath);
        
        // 更新提示信息，显示将要生成的文件
        updateExportHint();
    }
}

void ExportDialog::onFormatChanged()
{
    updatePreview();
    updateExportHint();
    
    // 对于多格式导出，不需要自动修改文件路径扩展名
    // 因为会生成多个不同扩展名的文件
}

void ExportDialog::updatePreview()
{
    QStringList previews;
    QDateTime sampleTime = QDateTime::currentDateTime();
    
    // 为每种选中的格式生成预览
    if (txtCheckBox->isChecked()) {
        QStringList parts;
        if (timestampCheckBox->isChecked()) parts << sampleTime.toString("yyyy-MM-dd hh:mm:ss");
        if (levelCheckBox->isChecked()) parts << "[INFO]";
        if (moduleCheckBox->isChecked()) parts << "<系统>";
        if (contentCheckBox->isChecked()) parts << "这是一条示例日志内容";
        previews << "TXT: " + parts.join(" ");
    }
    
    if (csvCheckBox->isChecked()) {
        QStringList headers, values;
        if (timestampCheckBox->isChecked()) {
            headers << "时间戳";
            values << sampleTime.toString("yyyy-MM-dd hh:mm:ss");
        }
        if (levelCheckBox->isChecked()) {
            headers << "日志等级";
            values << "INFO";
        }
        if (moduleCheckBox->isChecked()) {
            headers << "模块";
            values << "系统";
        }
        if (contentCheckBox->isChecked()) {
            headers << "内容";
            values << "这是一条示例日志内容";
        }
        previews << "CSV: " + headers.join(",") + "\\n" + values.join(",");
    }
    
    if (jsonCheckBox->isChecked()) {
        QStringList fields;
        if (timestampCheckBox->isChecked()) 
            fields << QString("\"timestamp\": \"%1\"").arg(sampleTime.toString(Qt::ISODate));
        if (levelCheckBox->isChecked()) 
            fields << "\"level\": \"INFO\"";
        if (moduleCheckBox->isChecked()) 
            fields << "\"module\": \"系统\"";
        if (contentCheckBox->isChecked()) 
            fields << "\"content\": \"这是一条示例日志内容\"";
        previews << "JSON: {\\n  " + fields.join(",\\n  ") + "\\n}";
    }
    
    QString preview = previews.isEmpty() ? tr("请选择至少一种导出格式") : previews.join("\\n\\n");
    previewLabel->setText(preview);
}

void ExportDialog::onAccept()
{
    // 验证格式选择
    if (!txtCheckBox->isChecked() && !csvCheckBox->isChecked() && !jsonCheckBox->isChecked()) {
        QMessageBox::warning(this, tr("警告"), tr("请至少选择一种导出格式！"));
        return;
    }
    
    // 验证字段选择
    if (!timestampCheckBox->isChecked() && !levelCheckBox->isChecked() && 
        !moduleCheckBox->isChecked() && !contentCheckBox->isChecked()) {
        QMessageBox::warning(this, tr("警告"), tr("请至少选择一个要导出的字段！"));
        return;
    }
    
    // 验证导出目录
    if (filePathLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择导出文件夹！"));
        return;
    }
    
    // 保存设置
    saveSettings();
    
    accept();
}



void ExportDialog::loadSettings()
{
    // 恢复格式选择
    QStringList lastFormats = AppSettings::instance().getLastExportFormats();
    txtCheckBox->setChecked(lastFormats.contains("TXT"));
    csvCheckBox->setChecked(lastFormats.contains("CSV"));
    jsonCheckBox->setChecked(lastFormats.contains("JSON"));
    
    // 如果没有选择任何格式，默认选择TXT
    if (!txtCheckBox->isChecked() && !csvCheckBox->isChecked() && !jsonCheckBox->isChecked()) {
        txtCheckBox->setChecked(true);
    }
    
    // 恢复字段选择
    bool timestamp, level, module, content;
    AppSettings::instance().getLastIncludeFields(timestamp, level, module, content);
    timestampCheckBox->setChecked(timestamp);
    levelCheckBox->setChecked(level);
    moduleCheckBox->setChecked(module);
    contentCheckBox->setChecked(content);
    
    // 更新提示文字
    updateExportHint();
}

void ExportDialog::saveSettings()
{
    // 保存格式选择
    QStringList formats;
    if (txtCheckBox->isChecked()) formats << "TXT";
    if (csvCheckBox->isChecked()) formats << "CSV";
    if (jsonCheckBox->isChecked()) formats << "JSON";
    AppSettings::instance().setLastExportFormats(formats);
    
    // 保存字段选择
    AppSettings::instance().setLastIncludeFields(
        timestampCheckBox->isChecked(),
        levelCheckBox->isChecked(),
        moduleCheckBox->isChecked(),
        contentCheckBox->isChecked()
    );
}

QString ExportDialog::getDefaultFileName() const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return QString("日志导出_%1").arg(timestamp);
}

QString ExportDialog::getFormatFilter() const
{
    return tr("所有文件 (*.*)");
}

QStringList ExportDialog::getSelectedFormats() const
{
    QStringList formats;
    if (txtCheckBox->isChecked()) formats << "TXT";
    if (csvCheckBox->isChecked()) formats << "CSV";
    if (jsonCheckBox->isChecked()) formats << "JSON";
    return formats;
}

QString ExportDialog::getExtensionForFormat(const QString& format) const
{
    if (format == "TXT") return ".txt";
    if (format == "CSV") return ".csv";
    if (format == "JSON") return ".json";
    return ".txt";
}

void ExportDialog::updateExportHint()
{
    QStringList selectedFormats = getSelectedFormats();
    QString selectedDir = filePathLineEdit->text();
    QString defaultFileName = getDefaultFileName();
    
    if (selectedFormats.isEmpty()) {
        exportHintLabel->setText(tr("请先选择至少一种导出格式"));
    } else {
        QString hint;
        
        if (selectedDir.isEmpty()) {
            hint = tr("请选择导出文件夹，将自动生成以下文件：\n");
        } else {
            hint = tr("将在选定文件夹中生成以下文件：\n");
        }
        
        for (const QString& format : selectedFormats) {
            QString ext = getExtensionForFormat(format);
            QString fileName = defaultFileName + ext;
            
            if (!selectedDir.isEmpty()) {
                // 显示完整路径
                QString fullPath = QDir(selectedDir).filePath(fileName);
                hint += QString("• %1\n").arg(QFileInfo(fullPath).fileName());
            } else {
                // 只显示文件名模式
                hint += QString("• %1\n").arg(fileName);
            }
        }
        
        exportHintLabel->setText(hint.trimmed());
    }
} 