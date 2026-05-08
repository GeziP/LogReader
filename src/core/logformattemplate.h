#ifndef LOGFORMATTEMPLATE_H
#define LOGFORMATTEMPLATE_H

#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

class LogFormatTemplate {
public:
    struct Preset {
        QString name;
        QString templateStr;
    };

    LogFormatTemplate();
    explicit LogFormatTemplate(const QString& templateStr);

    void setTemplate(const QString& templateStr);
    QString templateString() const;
    QRegularExpression regex() const;
    int captureIndex(const QString& fieldName) const;
    bool isValid() const;
    QString errorMessage() const;

    QStringList allFieldNames() const;
    QStringList extraFieldNames() const;

    static LogFormatTemplate detect(const QStringList& sampleLines);
    static QList<Preset> presets();
    static QString analyzeLineStructure(const QStringList& lines);
    static const QString DEFAULT_TEMPLATE;
    static const QStringList KNOWN_FIELDS;

private:
    void compile();

    QString m_template;
    QRegularExpression m_regex;
    QMap<QString, int> m_captureMap;
    bool m_valid;
    QString m_errorMessage;
};

#endif // LOGFORMATTEMPLATE_H
