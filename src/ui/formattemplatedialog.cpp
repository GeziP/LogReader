#include "formattemplatedialog.h"

#include "../core/logformattemplate.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

FormatTemplateDialog::FormatTemplateDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle(tr("Log Format Template"));
    resize(500, 350);

    debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);
    debounceTimer->setInterval(200);
    connect(debounceTimer, &QTimer::timeout, this,
            &FormatTemplateDialog::onPreviewUpdate);

    // Load presets
    const auto presets = LogFormatTemplate::presets();
    for (const auto& preset : presets) {
        presetCombo->addItem(preset.name, preset.templateStr);
    }
    presetCombo->addItem(tr("Custom"), QString());
}

void FormatTemplateDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // Preset selection
    auto* presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel(tr("Preset:")));
    presetCombo = new QComboBox();
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormatTemplateDialog::onPresetChanged);
    presetLayout->addWidget(presetCombo);
    mainLayout->addLayout(presetLayout);

    // Template edit
    auto* templateLayout = new QHBoxLayout();
    templateLayout->addWidget(new QLabel(tr("Template:")));
    templateEdit = new QLineEdit();
    connect(templateEdit, &QLineEdit::textChanged, this,
            &FormatTemplateDialog::onTemplateChanged);
    templateLayout->addWidget(templateEdit);
    mainLayout->addLayout(templateLayout);

    // Error label
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);
    mainLayout->addWidget(errorLabel);

    // Placeholder help
    auto* helpLabel = new QLabel(
        tr("Placeholders: {timestamp} {level} {module} {message}\n"
           "Escape literal braces with \\{ and \\}"));
    helpLabel->setStyleSheet("color: gray; font-size: 11px;");
    mainLayout->addWidget(helpLabel);

    // Preview section
    mainLayout->addWidget(new QLabel(tr("Preview:")));
    auto* sampleLayout = new QHBoxLayout();
    sampleLayout->addWidget(new QLabel(tr("Sample:")));
    sampleEdit = new QLineEdit();
    sampleEdit->setPlaceholderText(tr("Paste a log line here"));
    connect(sampleEdit, &QLineEdit::textChanged, this,
            &FormatTemplateDialog::onTemplateChanged);
    sampleLayout->addWidget(sampleEdit);
    mainLayout->addLayout(sampleLayout);

    resultLabel = new QLabel();
    resultLabel->setWordWrap(true);
    resultLabel->setMinimumHeight(60);
    mainLayout->addWidget(resultLabel);

    mainLayout->addStretch();

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    auto* okButton = new QPushButton(tr("OK"));
    auto* cancelButton = new QPushButton(tr("Cancel"));
    connect(okButton, &QPushButton::clicked, this,
            &FormatTemplateDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

void FormatTemplateDialog::setTemplate(const QString& templateStr)
{
    templateEdit->setText(templateStr);

    // Try to match a preset
    const auto presets = LogFormatTemplate::presets();
    for (int i = 0; i < presets.size(); ++i) {
        if (presets[i].templateStr == templateStr) {
            presetCombo->setCurrentIndex(i);
            return;
        }
    }
    presetCombo->setCurrentIndex(presetCombo->count() - 1); // Custom
}

QString FormatTemplateDialog::getTemplate() const
{
    return templateEdit->text().trimmed();
}

void FormatTemplateDialog::onPresetChanged(int index)
{
    QString templateStr = presetCombo->itemData(index).toString();
    if (!templateStr.isEmpty()) {
        templateEdit->setText(templateStr);
    }
}

void FormatTemplateDialog::onTemplateChanged()
{
    debounceTimer->start();
}

void FormatTemplateDialog::onPreviewUpdate()
{
    updatePreview();
}

void FormatTemplateDialog::updatePreview()
{
    QString templateStr = templateEdit->text().trimmed();
    if (templateStr.isEmpty()) {
        errorLabel->setVisible(false);
        resultLabel->clear();
        return;
    }

    LogFormatTemplate fmt(templateStr);
    if (!fmt.isValid()) {
        errorLabel->setText(fmt.errorMessage());
        errorLabel->setVisible(true);
        resultLabel->clear();
        return;
    }

    errorLabel->setVisible(false);

    QString sample = sampleEdit->text();
    if (sample.isEmpty()) {
        resultLabel->setText(tr("Enter a sample log line to preview"));
        return;
    }

    QRegularExpressionMatch match = fmt.regex().match(sample);
    if (!match.hasMatch()) {
        resultLabel->setStyleSheet("color: red;");
        resultLabel->setText(tr("No match"));
        return;
    }

    resultLabel->setStyleSheet("color: green;");
    QStringList parts;
    int tsIdx = fmt.captureIndex("timestamp");
    int lvIdx = fmt.captureIndex("level");
    int modIdx = fmt.captureIndex("module");
    int msgIdx = fmt.captureIndex("message");

    if (tsIdx >= 0)
        parts << tr("timestamp=%1").arg(match.captured(tsIdx).trimmed());
    if (lvIdx >= 0)
        parts << tr("level=%1").arg(match.captured(lvIdx).trimmed());
    if (modIdx >= 0)
        parts << tr("module=%1").arg(match.captured(modIdx).trimmed());
    if (msgIdx >= 0)
        parts << tr("message=%1").arg(match.captured(msgIdx).trimmed());

    resultLabel->setText(parts.join("  |  "));
}

void FormatTemplateDialog::onAccept()
{
    QString templateStr = templateEdit->text().trimmed();
    if (templateStr.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Template cannot be empty"));
        return;
    }

    LogFormatTemplate fmt(templateStr);
    if (!fmt.isValid()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Invalid template: %1")
                                 .arg(fmt.errorMessage()));
        return;
    }

    accept();
}
