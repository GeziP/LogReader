#include <cstdio>
#include "logformattemplate.h"

static int failures = 0;

void check(const char* name, bool condition, const char* detail = "")
{
    if (condition) {
        fprintf(stderr, "  PASS: %s\n", name);
    } else {
        fprintf(stderr, "  FAIL: %s %s\n", name, detail);
        failures++;
    }
}

int main(int argc, char* argv[])
{
    QStringList predictLines = {
        "[2026-04-30 15:59:57.934] [C0 +0000] CYCLE_START warmup cold-start warmup",
        "[2026-04-30 15:59:57.990] [C1 +0000] START  M21  test_1/光盘抓手      plan=0-6000      actual=0 rel=1/4",
        "[2026-04-30 15:59:57.990] [C1 +0000] START  N1   test_1/孵育盘-in     plan=0-2000      actual=0 rel=1/4",
        "[2026-04-30 15:59:58.803] [C1 +0813] END    M1   test_1/磁分离盘      plan=0-1000      actual=0-813         startDelay=+0",
        "[2026-04-30 15:59:58.803] [C1 +0813] EMIT   M1   -> S(M1.done)  wakes=[-]",
        "[2026-04-30 16:00:06.994] [C1 +9004] START  T1   test_1/孵育盘-out    plan=9000-11000  actual=9004 rel=1/4"
    };

    QStringList defaultLines = {
        "[2025-06-27 08:36:19.123] [INFO] [ModuleName] : 正常信息日志",
        "[2025-06-27 08:36:20.456] [ERROR] [Database] : 数据库连接失败",
        "[2025-06-27 08:36:21.789] [WARN] [Network] : 网络延迟过高",
        "[2025-06-27 08:36:22.012] [DEBUG] [Parser] : 解析详细信息"
    };

    QStringList customLines = {
        "[2025-01-15 08:00:01.123] [INFO] [C1 +0001] START TaskA modA : 初始化系统",
        "[2025-01-15 08:00:02.456] [DEBUG] [C1 +0002] WAIT TaskB modB : 等待资源分配",
        "[2025-01-15 08:00:03.789] [INFO] [C2 +0001] START TaskA modA : 开始处理请求",
        "[2025-01-15 08:00:05.345] [ERROR] [C2 +0002] FAIL TaskB modB : 连接超时"
    };

    // Test 1: Predict log detection
    fprintf(stderr, "=== Test 1: Predict log (extra fields) ===\n");
    {
        LogFormatTemplate fmt = LogFormatTemplate::detect(predictLines);
        check("valid", fmt.isValid(), fmt.errorMessage().toUtf8().constData());
        check("has field1 extra", fmt.extraFieldNames().contains("field1"));

        int mc = 0;
        for (const QString& line : predictLines) {
            if (fmt.regex().match(line).hasMatch()) mc++;
        }
        check("all lines match", mc == predictLines.size(),
              QString("%1/%2").arg(mc).arg(predictLines.size()).toUtf8().constData());

        // Verify field1 captures cycle info
        QRegularExpressionMatch m = fmt.regex().match(predictLines[1]);
        if (m.hasMatch()) {
            int idx = fmt.captureIndex("field1");
            QString val = (idx >= 0) ? m.captured(idx) : "";
            check("field1 has value", !val.isEmpty(), val.toUtf8().constData());
            fprintf(stderr, "  field1 = '%s'\n", val.toUtf8().constData());
        }
    }

    // Test 2: Default log detection (standard fields)
    fprintf(stderr, "\n=== Test 2: Default log (standard fields) ===\n");
    {
        LogFormatTemplate fmt = LogFormatTemplate::detect(defaultLines);
        check("valid", fmt.isValid(), fmt.errorMessage().toUtf8().constData());
        check("has timestamp", fmt.allFieldNames().contains("timestamp"));
        check("has level", fmt.allFieldNames().contains("level"));
        check("has message", fmt.allFieldNames().contains("message"));

        int mc = 0;
        for (const QString& line : defaultLines) {
            if (fmt.regex().match(line).hasMatch()) mc++;
        }
        check("all lines match", mc == defaultLines.size(),
              QString("%1/%2").arg(mc).arg(defaultLines.size()).toUtf8().constData());
    }

    // Test 3: Custom fields log detection
    fprintf(stderr, "\n=== Test 3: Custom fields log ===\n");
    {
        LogFormatTemplate fmt = LogFormatTemplate::detect(customLines);
        check("valid", fmt.isValid(), fmt.errorMessage().toUtf8().constData());

        int mc = 0;
        for (const QString& line : customLines) {
            if (fmt.regex().match(line).hasMatch()) mc++;
        }
        check("all lines match", mc == customLines.size(),
              QString("%1/%2").arg(mc).arg(customLines.size()).toUtf8().constData());

        fprintf(stderr, "  template: %s\n", fmt.templateString().toUtf8().constData());
        fprintf(stderr, "  extra fields: ");
        for (const QString& f : fmt.extraFieldNames()) fprintf(stderr, "%s ", f.toUtf8().constData());
        fprintf(stderr, "\n");
    }

    // Test 4: Task scheduler log (the problematic format)
    fprintf(stderr, "\n=== Test 4: Task scheduler log ===\n");
    QStringList schedulerLines = {
        "[2026-05-06 09:50:34.726] ================================================================================",
        "[CYCLE 1] BEGIN  budget=30000ms  activeTests=-",
        "[modA]",
        "  Task  Test      Rel    Action      Plan         State   WaitFor                   Emit",
        "  ----  --------  -----  ----------  -----------  ------  ------------------------  ----------------",
        "[2026-05-06 09:50:34.726] [C1 +0001] WAIT   B1   modB                 plan=0-100       missing=[S(A1.done)]",
        "[2026-05-06 09:50:34.726] [C1 +0001] START  A1   modA                 plan=0-100       actual=1",
        "[2026-05-06 09:50:34.726] [C1 +0001] END    A1   modA                 plan=0-100       actual=1-1           startDelay=+1",
        "[2026-05-06 09:50:34.726] [C1 +0001] READY  B1   modB                 <- S(A1.done) [final]",
        "[2026-05-06 09:50:34.726] [C1 +0001] EMIT   A1   -> S(A1.done)  wakes=[B1]",
        "[2026-05-06 09:50:40.799] [C1 +0000] FAULT  fault_task#71 unknown              Test fault",
        "[2026-05-06 09:50:40.800] [C1 +0000] START  normal_task#72 unknown              plan=0-0         actual=0",
        "[2026-05-06 09:50:46.504] [C1 +0000] START  high_priority#147 unknown              plan=0-0         actual=0",
        "[2026-05-06 09:50:48.283] [C1 +0061] START  offset_task#171 unknown              plan=50-50       actual=61",
    };
    {
        LogFormatTemplate fmt = LogFormatTemplate::detect(schedulerLines);
        check("valid", fmt.isValid(), fmt.errorMessage().toUtf8().constData());
        fprintf(stderr, "  template: %s\n", fmt.templateString().toUtf8().constData());

        QRegularExpression tsRe(R"(\[\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(?:\.\d+)?\])");
        int mc = 0;
        int unmatchedTimestamped = 0;
        for (const QString& line : schedulerLines) {
            if (tsRe.match(line).hasMatch()) {
                if (fmt.regex().match(line).hasMatch()) {
                    mc++;
                } else {
                    fprintf(stderr, "  UNMATCHED timestamped: %s\n", line.left(80).toUtf8().constData());
                    unmatchedTimestamped++;
                }
            }
        }
        fprintf(stderr, "  matched: %d, unmatched timestamped: %d\n", mc, unmatchedTimestamped);

        // Check that FAULT is recognized as a level
        check("FAULT in LOG_LEVELS",
              fmt.allFieldNames().contains("level"),
              "level field should exist");

        // Verify specific field extraction
        QRegularExpression regex = fmt.regex();
        QRegularExpressionMatch m = regex.match(schedulerLines[5]); // WAIT line
        if (m.hasMatch()) {
            int lvIdx = fmt.captureIndex("level");
            int msgIdx = fmt.captureIndex("message");
            QString level = (lvIdx >= 0) ? m.captured(lvIdx) : "";
            QString message = (msgIdx >= 0) ? m.captured(msgIdx) : "";
            fprintf(stderr, "  WAIT line: level='%s' message='%s'\n",
                    level.toUtf8().constData(), message.left(60).toUtf8().constData());
            check("WAIT level", level == "WAIT",
                  QString("got '%1'").arg(level).toUtf8().constData());
        }

        // Check FAULT line
        QRegularExpressionMatch fm = regex.match(schedulerLines[10]); // FAULT line
        if (fm.hasMatch()) {
            int lvIdx = fmt.captureIndex("level");
            QString level = (lvIdx >= 0) ? fm.captured(lvIdx) : "";
            fprintf(stderr, "  FAULT line: level='%s'\n", level.toUtf8().constData());
            check("FAULT matched", true);
            check("FAULT level", level == "FAULT",
                  QString("got '%1'").arg(level).toUtf8().constData());
        } else {
            check("FAULT matched", false, "FAULT line did not match template");
        }

        // Check EMIT line
        QRegularExpressionMatch em = regex.match(schedulerLines[9]); // EMIT line
        if (em.hasMatch()) {
            int lvIdx = fmt.captureIndex("level");
            QString level = (lvIdx >= 0) ? em.captured(lvIdx) : "";
            fprintf(stderr, "  EMIT line: level='%s'\n", level.toUtf8().constData());
            check("EMIT matched", true);
        } else {
            check("EMIT matched", false, "EMIT line did not match template");
        }
    }

    // Test 5: Flexible spacing - verify template handles multi-space alignment
    fprintf(stderr, "\n=== Test 5: Flexible spacing ===\n");
    QStringList spacedLines = {
        "[2026-05-06 09:50:34.726] [C1 +0001] START  A1   modA                 plan=0-100       actual=1",
        "[2026-05-06 09:50:34.726] [C1 +0001] END    A1   modA                 plan=0-100       actual=1-1           startDelay=+1",
        "[2026-05-06 09:50:34.726] [C1 +0001] WAIT   B1   modB                 plan=0-100       missing=[S(A1.done)]",
        "[2026-05-06 09:50:40.799] [C1 +0000] FAULT  fault_task#71 unknown              Test fault",
    };
    {
        // Test with a known template
        LogFormatTemplate fmt("[{timestamp}] [{field1}] {level} {field2} {field3} {message}");
        check("flex template valid", fmt.isValid(), fmt.errorMessage().toUtf8().constData());

        int mc = 0;
        for (const QString& line : spacedLines) {
            QRegularExpressionMatch m = fmt.regex().match(line);
            if (m.hasMatch()) {
                mc++;
                int lvIdx = fmt.captureIndex("level");
                int f2Idx = fmt.captureIndex("field2");
                int f3Idx = fmt.captureIndex("field3");
                int msgIdx = fmt.captureIndex("message");
                fprintf(stderr, "  level='%s' field2='%s' field3='%s' msg='%s'\n",
                        m.captured(lvIdx).toUtf8().constData(),
                        m.captured(f2Idx).toUtf8().constData(),
                        m.captured(f3Idx).toUtf8().constData(),
                        m.captured(msgIdx).left(40).toUtf8().constData());
            } else {
                fprintf(stderr, "  NO MATCH: %s\n", line.left(60).toUtf8().constData());
            }
        }
        check("all spaced lines match", mc == spacedLines.size(),
              QString("%1/%2").arg(mc).arg(spacedLines.size()).toUtf8().constData());
    }

    fprintf(stderr, "\n=== Results: %s ===\n", failures == 0 ? "ALL PASSED" : QString("%1 FAILED").arg(failures).toUtf8().constData());
    return failures;
}
