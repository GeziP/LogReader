# CI一致性与架构适配优化任务

## 背景
- 解决CI与本地clang-format风格不一致、Qt库与runner架构不匹配导致的构建/链接失败。
- 目标：CI与本地工具链完全一致，CI自动适配各平台架构，文档同步说明。

## 主要操作
1. 检查并确认项目根目录唯一`.clang-format`，CI与本地强制统一风格。
2. CI脚本（quality.yml/release.yml）全部Qt安装步骤补充`arch`参数：
   - Windows: `arch: windows_x64`
   - Linux: `arch: linux_x64`
   - macOS: `arch: clang_64`
3. 文档（中英文CI/CD指南）补充格式化工具链、Qt架构适配说明及常见问题排查建议。

## 结果
- CI与本地clang-format风格完全一致，格式误报消除。
- CI自动适配各平台Qt架构，彻底解决"架构不匹配"链接失败。
- 文档同步，开发者易于理解和排查。

## 建议
- 后续如需支持新平台，扩展`arch`参数即可。
- 开发者本地务必用与CI一致的clang-format版本。
- 若遇架构相关CI失败，优先检查Qt安装arch参数与runner平台。 