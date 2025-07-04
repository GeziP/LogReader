# CI/CD 流程质量提升

## 背景
当前项目已具有跨平台 Release 构建工作流 `release.yml`，但缺乏持续的质量保障机制（静态分析、动态分析、覆盖率、安全扫描等）。为提高代码质量、符合 C++ Core Guidelines，并方便在 Pull Request 阶段及时发现问题，需要引入独立的 Quality Workflow 及配套工具。

## 目标
1. 引入静态代码分析（clang-tidy、cppcheck）并启用 `cppcoreguidelines-*` 规则集。
2. 引入代码格式检查（clang-format）确保统一编码风格。
3. 引入动态运行时检查（ASAN / UBSAN）发现潜在内存与未定义行为。
4. 生成并上报代码覆盖率（gcov + lcov → Codecov badge）。
5. 安全与依赖扫描（Trivy、GitHub Secret Scanning）。
6. 集成 **Guidelines Support Library (GSL)** 并在静态分析中检测其正确使用。
7. 通过 **pre-commit** 钩子在本地阻止不合规提交。
8. 保持与现有 `release.yml` 构建流程解耦，互不影响。

## 任务清单
- [ ] 创建 `.github/workflows/quality.yml`，在 `push` / `pull_request` 触发。
  - [ ] Matrix 运行：`ubuntu-latest`, `windows-latest`, `macos-latest`。
  - [ ] Job: **Build & Test**（Debug）
  - [ ] Job: **Static-Analysis** — clang-tidy、cppcheck、clang-format diff。
  - [ ] Job: **Sanitizers** — Linux/macOS 启用 ASAN & UBSAN。
  - [ ] Job: **Coverage** — 生成 lcov 报告并上传 Codecov。
  - [ ] Job: **Security-Scan** — Trivy 并启用 GitHub 内置 Secret Scanning。
- [ ] 新增 `.clang-tidy` 配置文件，启用 `cppcoreguidelines-*`, `modernize-*`, `performance-*`, `readability-*` 等。
- [ ] 新增 `.clang-format`（基于 LLVM 风格，80 列宽）。
- [ ] 编写 `scripts/run_cppcheck.sh` 统一 cppcheck 调用并输出 XML 供 CI 解析。
- [ ] 通过 vcpkg / git submodule 引入 `microsoft-gsl`，并在 `CMakeLists.txt`、`LogReader.pro` 中链接。
- [ ] 添加 `.pre-commit-config.yaml`，集成 clang-format、clang-tidy、cppcheck、cmake-format。
- [ ] 更新 `docs/CI_CD_GUIDE.md` 说明质量流程与本地开发指南。

## 验收标准
1. 新增 **Quality** 工作流在每个 Pull Request 自动运行，并在状态检查中展示。
2. 静态分析 **0 Error / 0 Warning**（或预设阈值）方可合并。
3. 动态运行时检查 100% 通过，无崩溃或 Undefined Behavior 报告。
4. 覆盖率报告成功上传，主分支 badge ≥ 80%。
5. 所有依赖与容器扫描无高危漏洞。

## 里程碑与迭代
- **v1.3.0**：引入 clang-tidy、clang-format、质量工作流骨架。
- **v1.4.0**：覆盖 cppcheck、ASAN/UBSAN、覆盖率上传。
- **v1.5.0**：集成 GSL、pre-commit、完整安全扫描。

## 参考链接
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- Microsoft GSL: https://github.com/microsoft/GSL
- clang-tidy docs: https://clang.llvm.org/extra/clang-tidy/
- cppcheck: https://github.com/danmar/cppcheck
- ASAN/UBSAN: https://clang.llvm.org/docs/AddressSanitizer.html
- Codecov: https://about.codecov.io
- Trivy: https://github.com/aquasecurity/trivy

---
**负责人**: @GeziP  
**优先级**: High  
**状态**: Draft 