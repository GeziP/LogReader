#ifndef FORMATEMPLATEDIALOG_H
#define FORMATEMPLATEDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

class FormatTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormatTemplateDialog(QWidget* parent = nullptr);

    void setTemplate(const QString& templateStr);
    QString getTemplate() const;

private slots:
    void onPresetChanged(int index);
    void onTemplateChanged();
    void onPreviewUpdate();
    void onAccept();

private:
    void setupUI();
    void updatePreview();

    QComboBox* presetCombo;
    QLineEdit* templateEdit;
    QLineEdit* sampleEdit;
    QLabel* resultLabel;
    QLabel* errorLabel;
    QTimer* debounceTimer;
};

#endif // FORMATEMPLATEDIALOG_H
