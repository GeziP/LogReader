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

    fprintf(stderr, "\n=== Results: %s ===\n", failures == 0 ? "ALL PASSED" : QString("%1 FAILED").arg(failures).toUtf8().constData());
    return failures;
}
