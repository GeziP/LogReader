---
title: C++ 代码规范与实践（基于 clang 工具链）
audience: 新人培训 / 代码评审
version: 1.0
---

> 说明：本文基于基准项目模板 [cpp-best-practices/cmake_template](https://github.com/cpp-best-practices/cmake_template) 编写。文中提到的“本模板”均指该模板；未来我们会整理并发布公司的自有模板工程，届时会同步更新本文说明与适配细节。
>
> 风格基线：统一对齐 [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)，并以 clang-format 的 [LLVM 风格选项](https://clang.llvm.org/docs/ClangFormatStyleOptions.html) 为实现依据（`BasedOnStyle: LLVM`）。

## 目的

- 用最小记忆成本统一风格，提高评审效率与代码长期可维护性。
- 基于 `.clang-format` 与 `.clang-tidy` 的各两套风格：快速迭代版（fast）与高质量版（quality）。
- 本文详细说明每一条规则的作用、限制、默认值与示例，并给出命名、异常/错误处理、注释规范等工程实践。

## 1. 如何使用
- 构建：已启用 `compile_commands.json`，CLion/VSCode 可自动识别。是否启用 `clang-tidy` 由 CMake 变量 `myproject_ENABLE_CLANG_TIDY` 控制（顶层默认启用）。
- 范围：默认检查 `include/` 与 `src/`。第三方依赖与生成目录不在检查范围内。

## 2. clang-format 规则详解

> 参考：风格基线对齐 [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)；clang-format 具体选项见 [ClangFormat Style Options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)。

### 总览
- fast：偏宽松，减少自动格式化产生的大规模 diff，提升迭代效率。
- quality：偏严格，强调可读性与一致性，适合核心库或稳定代码。

### 约定
- 未在下文列出的条目，沿用 `BasedOnStyle: LLVM` 默认值。
- 示例使用对比方式展示“建议”与“不建议”。

### 1. 缩进与指针引用对齐
- IndentWidth: 4 / TabWidth: 4 / UseTab: Never（fast/quality 一致）
  - 作用：统一使用 4 空格缩进，避免制表符导致的跨平台显示差异。
  - 示例：
    ```cpp
    if (ok) 
    {
        do_work();
    }
    ```
- ContinuationIndentWidth: 4（fast/quality 一致）
  - 作用：续行缩进 4，兼顾紧凑与可读。
- PointerAlignment: Left / ReferenceAlignment: Left（fast/quality 一致）
  - 作用：`int* p; int& r;` 将符号靠近类型，减少错读。
  - 对比：
    - 建议：`int* p; int& r;`
    - 不建议：`int *p; int &r;`

### 2. 行宽与换行
- ColumnLimit: 120（fast）/ 100（quality）
  - 作用：限制行宽。质量版更严格，便于并排 diff 与评审。
  - 建议：超过行宽的长表达式与参数列表进行换行。

### 3. 大括号风格（统一为 Allman）
- BreakBeforeBraces: Allman（fast/quality 一致）
  - 示例：
    ```cpp
    if (ready)
    {
        run();
    }
    ```

### 4. 短语句与单行容忍度
- fast：
  - AllowShortIfStatementsOnASingleLine: AllIfsAndElse
  - AllowShortFunctionsOnASingleLine: Inline
  - AllowShortLoopsOnASingleLine: true
  - 作用：允许极短 if/函数/循环单行，提高编辑速度。
- quality：
  - AllowShortIfStatementsOnASingleLine: Never
  - AllowShortFunctionsOnASingleLine: None
  - AllowShortLoopsOnASingleLine: false
  - 作用：强制多行与大括号，降低误读与“悬挂 else”风险。
  - 示例：
    - 建议：
      ```cpp
      if (ok) {
          return 1;
      }
      ```
    - 不建议：`if (ok) return 1;`

### 5. 参数打包与构造初始化
- BinPackParameters / BinPackArguments: true（fast）/ false（quality）
  - fast：尽量一行放下更多实参，减少换行。
  - quality：一行一项，有利于最小 diff 与逐项评审。
- ConstructorInitializerAllOnOneLineOrOnePerLine: true（fast/quality 一致）
  - 作用：构造初始化要么全在一行，要么一项一行，避免混排。

### 6. 括号前空格
- SpaceBeforeParens: ControlStatements（fast）/ Always（quality）
  - fast：仅 `if/for/while` 前加空格，函数调用不加：`if (x) f(x);`
  - quality：统一前置空格：`if (x)`、`f (x)`。
  - 说明：质量版牺牲部分紧凑性，换取一致性。

### 7. 空行与注释重排
- KeepEmptyLinesAtTheStartOfBlocks: true（fast）/ false（quality）
  - fast：允许块起始保留空行，兼容存量代码。
  - quality：清理块起始空行，增强紧凑与一致性。
- ReflowComments: false（fast）/ true（quality）
  - fast：不自动重排注释，减少无谓改动。
  - quality：自动重排，确保注释遵守行宽，利于阅读。

### 8. include 管理
- SortIncludes: true（fast/quality 一致）
- IncludeBlocks: Preserve（fast）/ Regroup（quality）
  - fast：保持 include 原有分组，仅在组内排序；改动面小。
  - quality：强制分组重排，体现依赖层次。
- IncludeCategories（fast/quality 一致）
  1) `<...>` 系统/第三方头（Priority 1）
  2) `"myproject/..."` 项目公共头（Priority 2）
  3) 其他 `"..."` 本地头（Priority 3）
- SortUsingDeclarations: true（fast/quality 一致）

8.1 IncludeCategories 细分建议（可选）
- 目的：在质量版下进一步细分 include 分组，提升依赖层次的可读性。
- 建议分组（按优先级从小到大排序）：
  1) 系统与 C/C++ 标准库头：`^<[^"/]+>$`（如 `<vector>`、`<stdio.h>`）
  2) 常见第三方库：
     - `^<absl/.*>$`、`^<boost/.*>$`、`^<gtest/.*>$`、`^<gmock/.*>$`、`^<fmt/.*>$`
  3) 生成头文件：`^"configured_files/.*"$`
  4) 项目公共头：`^"myproject/.*"$`
  5) 本地相对路径：`^"[^/].*"$`
- 示例：
  ```cpp
  #include <vector>                     // 1) 标准库
  #include <absl/strings/str_split.h>   // 2) 第三方
  
  #include "configured_files/config.hpp" // 3) 生成
  #include "myproject/sample_library.hpp" // 4) 项目公共
  #include "internal/util.hpp"            // 5) 本地
  ```
- 说明：第三方库的正则请按实际依赖增补；分组只在质量版 `IncludeBlocks: Regroup` 下严格生效。

### 9. 对齐策略
- fast：
  - AlignConsecutiveAssignments/Declarations: false
  - AlignOperands: true
  - AlignTrailingComments: true
  - 解释：避免大面积对齐导致的噪音，保留操作数/尾注释的基本对齐。
- quality：
  - AlignConsecutiveAssignments/Declarations: true
  - AlignOperands: true
  - AlignTrailingComments: true
  - 解释：同类项纵向对齐，提升浏览速度，适合稳定代码。

### 10. 命名空间缩进
- NamespaceIndentation: None（fast）/ All（quality）
  - fast：兼容历史代码，命名空间体不额外缩进。
  - quality：命名空间体缩进一层，层次更清晰。

### 11. 其他（quality 专属）
- BreakBeforeBinaryOperators: NonAssignment
  - 作用：换行时，非赋值二元运算符前断行，阅读更自然。
- SpacesInParentheses: false
  - 作用：括号内不加空格：`f(x, y)`。

## 3. clang-tidy 规则详解与示例

### 总览
- fast：聚焦缺陷与现代化建议，避免重构级别的大动静；`WarningsAsErrors` 为空。
- quality：全面检查并对关键缺陷提升到错误；强化命名、可读性、安全与性能。

### 通用设置
- HeaderFilterRegex: `^(src|include)/`：限定检查目录范围。
- InheritParentConfig: false：不继承上层配置，防止环境干扰。
- AnalyzeTemporaryDtors：fast=false / quality=true：是否分析临时对象析构路径。
- FormatStyle: none（fast）：依赖 clang-format；quality 不设此项。

### fast 检查集合（.clang-tidy.fast）
- Checks（选摘）
  - `bugprone-*`：易错模式，如空指针、未使用返回值、错误大小计算等。
  - `clang-analyzer-*`：路径/数据流分析，捕获越界、未初始化、资源泄漏。
  - `modernize-*`：`nullptr/override/final/range-for/using` 等现代化建议。
  - `performance-*`：避免不必要拷贝、临时对象、重复构造等。
  - `readability-braces-around-statements`：鼓励补全大括号（短行放宽）。
  - `readability-implicit-bool-conversion`/`readability-redundant-declaration`/`readability-uniqueptr-delete-release`。
  - `cppcoreguidelines-narrowing-conversions/owning-memory/slicing`：收敛危险行为。
- CheckOptions
  - `modernize-use-auto.MinTypeNameLength: 5`
    - 作用：仅当类型名较长时建议使用 `auto`，减少可读性损失。
  - `modernize-use-nullptr.IgnoreMacros: true`、`NullMacros: NULL`
    - 作用：宏内不强制替换；兼容老代码。
  - `readability-braces-around-statements.ShortStatementLines: 1`
    - 作用：极短语句允许单行。
- 示例
  ```cpp
  char* p = NULL;              // 建议：使用 nullptr（modernize-use-nullptr）
  std::string* s = nullptr;    // 仍需注意解引用空指针风险（analyzer-null-dereference）
  
  for (int i = 0; i < v.size(); i++) { use(v[i]); } // 建议：基于范围的 for（modernize-loop-convert）
  ```

### quality 检查集合（.clang-tidy.quality）
- Checks（按类别）
  - `bugprone-*`、`clang-analyzer-*`（关键缺陷类，见下文错误处理）
  - `cert-*`、`hicpp-*`：行业通用的安全与规则集。
  - `cppcoreguidelines-*`：指导性规则，覆盖内存、接口、安全与现代化。
  - `modernize-*`、`performance-*`、`portability-*`、`readability-*`、`misc-*`。
- 关键项提升为错误
  - `WarningsAsErrors: 'bugprone-*,clang-analyzer-*'`
  - 说明：一旦触发，这类问题应在 PR 内修复或回退改动。
 - 命名规范（readability-identifier-naming）
   - 启用命名检查以配合团队约定；具体规范请参见“4. 命名规范（质量版约束，快速版参考）”。
- 其他选项
  - `modernize-use-auto.MinTypeNameLength: 5`
  - `cppcoreguidelines-avoid-magic-numbers.IgnoredNumericLiterals: 0,1,2`
  - `cppcoreguidelines-pro-type-member-init.IgnoreArrays: false`
- 示例
  ```cpp
  struct HttpRequest
  {
      std::string url_{};      // 私有成员后缀 `_`
      int timeout_ms_{};       // 成员初始化（cppcoreguidelines-pro-type-member-init）
  };
  
  int computeHash (std::string_view s); // SpaceBeforeParens: Always（quality 格式化）
  
  void f()
  {
      sleep(5); // 警告：magic number → 建议：constexpr std::chrono::seconds kDefaultSleep{5};
  }
  ```

2.1 performance-* 典型告警对比

- performance-unnecessary-copy-initialization
  - 反例：
    ```cpp
    std::string s = get_str(); // 若 get_str() 返回临时，复制一次
    ```
  - 建议：
    ```cpp
    std::string s = get_str();            // NRVO/移动消除复制
    const std::string s2 = get_str();     // 避免先拷贝再赋值
    ```

- performance-unnecessary-value-param
  - 反例：
    ```cpp
    void f(std::string s); // 实参被拷贝
    ```
  - 建议：
    ```cpp
    void f(const std::string& s);     // 只读传引用
    void g(std::string s);            // 若需持有副本，可保留值传递
    ```

- performance-for-range-copy
  - 反例：
    ```cpp
    for (auto x : big_vec) { use(x); } // 复制每个元素
    ```
  - 建议：
    ```cpp
    for (const auto& x : big_vec) { use(x); }
    ```

- performance-inefficient-string-concatenation
  - 反例：
    ```cpp
    std::string s;
    for (auto x : v) s += x; // 反复扩容
    ```
  - 建议：
    ```cpp
    std::string s;
    s.reserve(v.size());
    for (auto x : v) s += x;
    ```

- performance-move-const-arg
  - 反例：
    ```cpp
    void f(const std::string& s);
    std::string a;
    f(std::move(a)); // 对 const& 实参移动无效
    ```
  - 建议：
    ```cpp
    f(a); // 直接传引用
    ```

2.2 cppcoreguidelines 边界案例

- avoid-magic-numbers
  - 边界：协议/枚举映射/测试断言中出现的少量字面量可例外；配置中已允许 `0,1,2`。
  - 建议：语义化的重要常量使用 `constexpr` 命名。

- pro-type-member-init
  - 边界：POD 聚合在某些平台上默认初始化成本可接受；但为一致性仍建议显式初始化。

- narrowing-conversions
  - 边界：与外部协议/硬件寄存器打交道时可局部抑制（NOLINT），并在注释中标注理由。

- owning-memory
  - 建议：优先采用 `unique_ptr`/`shared_ptr`/`span` 等 RAII/视图类型；对外暴露原始指针需在文档中声明所有权约定。

## 4. 命名规范（质量版约束，快速版参考）

总则
- 命名体现“意图”，避免缩写；统一英文；禁止拼音。
- 避免类型泄露到名称（如 `vectorList`），命名关注“用途”。

具体规则
- 类型：`CamelCase`，如 `ImageDecoder`、`UserId`。
- 函数/方法：`camelBack`，如 `loadConfig`、`resolvePath`。
- 变量/参数：`lower_case`，如 `user_count`、`max_size`。
- 常量/枚举常量：`UPPER_CASE`，如 `DEFAULT_PORT`、`HTTP_OK`。
- 命名空间：`lower_case`，如 `net`、`fs`。
- 成员命名：私有成员后缀 `_`；静态成员前缀 `s_` 可选。
- 谨慎使用缩写：通用缩写必须在团队词汇表中登记（如 `id`, `url`, `rpc`）。

反例与修正
```cpp
struct a_b { int ID; };               // 反例
struct UserProfile { int id; };       // 建议

int GETVAL();                         // 反例
int getValue();                       // 建议
```

## 5. 异常与错误处理准则

> 说明：不同项目对错误传播可能有不同基线。以下提供两种推荐模式，请按项目约定选择并保持一致。

### 5.1 错误码优先模式（无异常）

- **适用场景**：性能敏感/低延迟路径、跨语言或硬件交互层、对异常语义存在分歧的团队。
- **约束与做法**：
  - API 使用 `expected`/`status`/`error_code` 返回错误，禁止抛异常。
  - 通过类型表达错误域与可恢复性（C++17/23 兼容）：
  ```cpp
  // 统一别名：优先使用标准 expected；不满足条件时回退到 tl::expected
  #if __has_include(<expected>) && (__cplusplus >= 202302L)
  #  include <expected>
  template <class T, class E>
  using expected = std::expected<T, E>;
  #else
  #  include <tl/expected.hpp>
  template <class T, class E>
  using expected = tl::expected<T, E>;
  #endif

  enum class LoadError { NotFound, InvalidFormat };
  expected<Data, LoadError> load(std::string_view p);
  ```
  - 调用方必须显式处理错误分支；在重要路径打印结构化日志。
  - RAII 保证资源释放；避免在错误分支出现泄漏。
- **优点**：控制流显式、可预测；跨边界更稳定。
- **缺点**：调用点增加样板代码；深层错误传递需要嵌套包装。

### 5.2 异常优先模式（带异常）

- **适用场景**：库/通用组件、业务聚合层、以可读性与异常安全为重点的代码。
- **约束与做法**：
  - 可抛出标准库异常与自定义异常；析构函数不抛异常。
  - 提供强异常安全保证：失败不改变可见状态。
  - 限制异常的边界：模块出口处捕获并转译为统一错误模型。
  - 对外 ABI/FFI 边界避免泄漏 C++ 异常，必要时捕获后转为错误码。
- **优点**：调用点整洁；深层错误可直达外层。
- **缺点**：隐藏控制流；非本地跳转可能影响理解与优化。

> 建议：若团队未统一，优先在同一子系统内保持一致；跨子系统通过门面/适配层做模式转换（异常↔错误码）。

选择策略
- 库与通用组件：更倾向使用异常/`expected` 暴露错误（C++23 使用 `std::expected`，C++17 建议 `tl::expected` 或上文条件别名），避免隐藏状态码；跨 ABI/模块边界要谨慎抛出异常。
- 服务端与性能敏感路径：返回错误码或 `expected<T, E>` 更可控；异常仅用于不可恢复错误。

统一做法
- 使用 RAII 管理资源，确保异常/错误路径安全。
- 不要在析构函数抛出异常；必要时使用 `noexcept` 并在失败时写日志。
- 提供强异常安全保证：操作失败不改变对象可见状态。
- 底层接口不依赖全局可变状态；错误通过返回值或异常显式传播。

示例：使用 expected（C++17/23 兼容）
```cpp
// 统一别名：优先使用标准 expected；不满足条件时回退到 tl::expected
#if __has_include(<expected>) && (__cplusplus >= 202302L)
#  include <expected>
template <class T, class E>
using expected = std::expected<T, E>;
#else
#  include <tl/expected.hpp>
template <class T, class E>
using expected = tl::expected<T, E>;
#endif

enum class LoadError { NotFound, InvalidFormat };

expected<std::string, LoadError> load_text(std::string_view path);

auto res = load_text("/etc/a.txt");
if (!res) {
    // 统一处理错误
}
```

> 引入说明（C++17）：若采用回退方案，请引入单头库 `tl::expected`。
> - vcpkg：`vcpkg install tl-expected`
> - 源码：[`TartanLlama/expected`](https://github.com/TartanLlama/expected)

## 6. 注释与文档规范

总则
- 注释解释“为什么”而非“做了什么”。禁止使用表情与口语化表达。
- 使用 Doxygen 风格为对外接口编写文档；内部实现只在必要处加简明注释。

文件头与版权（建议模板）
```cpp
// [模块] 简述
// [作者] 名字/团队
// [变更记录] YYYY-MM-DD: 变更说明
```

函数/类文档（Doxygen 示例）
```cpp
/// 计算字符串哈希
/// \param s 输入字符串
/// \return 哈希值
int computeHash(std::string_view s);
```

注释格式化
- 质量版开启 `ReflowComments: true`，注释会被自动按行宽重排。
- 代码块与注释之间保留一行空行，提升可读性。

## 7. 抑制策略与例外管理

局部抑制
- 当前行：`// NOLINT(<check-name>)`
- 下一行：`// NOLINTNEXTLINE(<check-name>)`
- 要求：附加理由标签，例如 `[rationale: 与第三方接口兼容]`。

文件/目录级别
- 调整 `.clang-tidy` 中的 `HeaderFilterRegex` 或在 CI 中过滤路径。
- 禁止在大范围目录上长期禁用核心缺陷类检查（`bugprone-*`、`clang-analyzer-*`）。

迁移建议
- 新代码即刻遵循；存量代码“按目录/模块分批”接入。
- 质量版建议在核心模块试点，稳定后推广。

## 8. CI 与本地工作流建议

本地
- 开发者可使用 fast 风格提高效率；提交前运行 `clang-format` 与 `clang-tidy` 自检。

CI（推荐）
- 对 `quality` 风格执行 `clang-tidy`，并将 `bugprone-*`、`clang-analyzer-*` 视为错误阻塞合入。
- `clang-format` 可设置为“仅报告差异”，由开发者修复后重推。

## 9. 附录：常见问题（FAQ）
1) 为什么选择两套风格？
   - 兼顾迭代效率与长期质量；不同阶段与模块可灵活选择。
2) 可以只用一种吗？
   - 可以。把选定风格复制为根目录 `.clang-format`/`.clang-tidy` 即可。
3) IDE 插件会冲突吗？
   - 建议统一使用 clang 工具。不兼容时以仓库根配置为准。

## 9.1 附：未选入条款说明（节选）

- AllowShortIfStatementsOnASingleLine（质量版设为 Never）
  - 未选入 fast 的严格限制，原因：提高迭代效率；但 tidy 中仍鼓励加大括号，避免悬挂 else 与合并错误。

- AlignConsecutive*（fast 关闭、quality 开启）
  - 未选入 fast：为减少大面积对齐造成的 diff 与维护成本；quality 开启以提升浏览速度。

- IncludeBlocks: Regroup（fast 未选入）
  - 未选入 fast：避免对历史代码产生大范围 include 重排；quality 进行强制分组，明确依赖层次。

- SpaceBeforeParens: Always（fast 未选入）
  - 未选入 fast：函数调用不加空格以保持紧凑与与常见 IDE 默认一致；quality 统一风格以提升一致性。

- AnalyzeTemporaryDtors（fast=false）
  - 原因：该分析可能带来一定的开销与告警密度；quality 开启以捕获更细的临时对象析构路径问题。

- readability-identifier-naming（fast 未开启）
  - 原因：命名重构通常影响范围大，适合在质量门槛更高的代码区域逐步推进。

- portability-*（fast 未开启）
  - 原因：跨平台问题在快速阶段可由代码评审与编译器覆盖先行；在质量版中启用以提前暴露。

- misc-*（部分规则在 fast 未启）
  - 原因：misc 类规则杂而广，可能产生较多非关键提醒；质量版中统一放开。

## 10. GCC 专项说明与 C++17 兼容

10.1 总体兼容性
- clang-format 与编译器无关：GCC/Clang/MSVC 均可使用。
- clang-tidy 基于 `compile_commands.json` 进行语义分析：
  - 本模板已启用该文件；使用 GCC 构建同样会生成。
  - 构建命令中的标准选项（如 `-std=c++17`）会通过 `-extra-arg=-std=c++17` 注入 clang-tidy（已在 CMake 配置中处理），从而避免提出超出标准版本的建议。
- 与 PCH 的兼容性：当使用非 Clang 编译器且启用 PCH 时，clang-tidy 不支持（模板已在这种情况下报错），建议保持 `myproject_ENABLE_PCH=OFF`。

10.2 在 GCC 下的使用建议
- 配置标准版本两种方式：
  - 构建时设置：`cmake -S . -B build -DCMAKE_CXX_STANDARD=17`
  - 或修改顶层 `CMakeLists.txt` 的默认标准（当前模板默认 23），将 `set(CMAKE_CXX_STANDARD 23)` 改为 `17`。
- 运行 tidy/format：
  - IDE 内集成或命令行均可；建议以 CI 统一校验。
- Sanitizer 提示：
  - Linux + GCC：ASan/UBSan 一般可用。
  - Windows + GCC/MinGW：ASan 支持不稳定，本模板在 Windows 下默认关闭 AddressSanitizer（见 `ProjectOptions.cmake` 推导）。

10.3 仅支持到 C++17 的影响与处理
- modernize 系列检查会依据 `-std` 自动判定是否给出 C++20 相关建议；在 `-std=c++17` 下通常不会建议 `std::span`/`std::format`。
- 如仍希望显式屏蔽 C++20+ 的建议，可在 `.clang-tidy.*` 中额外禁用：
  ```yaml
  Checks: >
    ..., -modernize-use-std-span, -modernize-use-std-format
  ```
- 质量版的命名/可读性/安全/性能检查与 C++17 兼容，无需更改。
- 可考虑移除对 C++20 相关格式/约定的团队期望（若有）并在培训中明确标准上限。

10.4 常见问题
- 问：GCC 下 clang-tidy 能否准确理解 GCC 专有扩展？
  - 答：多数情况下可工作；个别 GCC 扩展可能需要通过 `HeaderFilterRegex` 规避或在本地以 NOLINT 抑制。
- 问：不同平台的头文件搜索路径差异？
  - 答：clang-tidy 依赖 `compile_commands.json` 中的包含路径；请通过构建系统保证路径完整（模板已默认启用）。

## 11.测试代码命名与布局规范

11.1 目录与文件命名
- 目录：`test/` 或 `tests/`（模板为 `test/`）。
- 文件：对应被测单元命名+`_tests.cpp`，如 `user_service_tests.cpp`。
- 基准/性能测试另建 `bench/` 或在 `test/perf/` 分组。

11.2 测试命名
- 用例：`<被测单元>_<场景>_<期望>`，如 `ParseConfig_InvalidPath_ReturnsError`。
- 使用框架建议：`gtest`：`TEST(SuiteName, CaseName)` 与 `TEST_F/TEST_P`。

11.3 布局与依赖
- include 顺序同生产代码；尽量避免在测试中引入生产私有头，首选公共接口。
- 夹具（fixture）封装重复准备/清理逻辑；避免全局静态可变状态。
- 魔法数字：允许在断言中少量出现，但对于复用的阈值/参数请语义化。

11.4 可读性
- 每个测试函数控制在 30-50 行内；Given/When/Then 分段注释清晰表达意图。

## 12.宏命名与条件编译规范

12.1 宏命名
- 常量宏：`UPPER_CASE`，如 `MAX_BUFFER_SIZE`。
- 宏函数：`UPPER_CASE`，参数使用括号包裹：
  ```cpp
  #define SQUARE(x) ((x) * (x))
  ```
- 头文件包含保护首选 `#pragma once`；如需宏守卫使用项目前缀：`MYPROJECT_MODULE_FILE_HPP`。

12.2 条件编译
- 平台/编译器判定集中到适配层，避免在核心业务代码大量 `#ifdef`。
- 允许的判定宏举例：`_WIN32`、`__linux__`、`__APPLE__`、`__GNUC__`/`__clang__`。
- 与功能检测（feature-detect）优先：通过 CMake/配置头注入 `HAVE_xxx` 宏，避免直接判断编译器版本。
- 条件块内保持最小化，更复杂的分支放到独立实现文件。

## 13. 官方检查索引

> 提示：不同 clang-tidy 版本的检查集合会有差异，以下为官方文档入口，建议以本机版本为准。

- 官方总表：`https://clang.llvm.org/extra/clang-tidy/checks/list.html`
- clang-analyzer：`https://clang.llvm.org/extra/clang-tidy/checks/clang-analyzer.html`
- cppcoreguidelines：`https://clang.llvm.org/extra/clang-tidy/checks/cppcoreguidelines.html`
- modernize：`https://clang.llvm.org/extra/clang-tidy/checks/modernize.html`
- performance：`https://clang.llvm.org/extra/clang-tidy/checks/performance.html`
- readability：`https://clang.llvm.org/extra/clang-tidy/checks/readability.html`
- bugprone：`https://clang.llvm.org/extra/clang-tidy/checks/bugprone.html`
- cert：`https://clang.llvm.org/extra/clang-tidy/checks/cert.html`
- hicpp：`https://clang.llvm.org/extra/clang-tidy/checks/hicpp.html`
- portability：`https://clang.llvm.org/extra/clang-tidy/checks/portability.html`
- misc：`https://clang.llvm.org/extra/clang-tidy/checks/misc.html`

## 14. 常用检查速查表

> 说明：以下为实践中常用且价值高的子集，命名以官方为准；详细参数与边界请查阅上方链接。

- bugprone（易出错）
  - bugprone-branch-clone：分支体重复，可能是复制粘贴失误。
  - bugprone-macro-parentheses：宏参数缺少括号，易产生优先级错误。
  - bugprone-use-after-move：对象 move 后继续使用。
  - bugprone-sizeof-expression：对表达式/类型误用 sizeof，结果非预期。
  - bugprone-signed-char-misuse：有符号 char 被用作字节处理，导致符号扩展风险。
  - bugprone-virtual-near-miss：虚函数签名近似但未覆盖，易造成逻辑遗漏。
  - bugprone-undefined-memory-manipulation：对未定义内存做拷贝/比较。
  - bugprone-string-constructor：`std::string(ptr, len)` 与 `strlen`/含 0 字节的陷阱。
  - bugprone-integer-division：整数相除被截断。
  - bugprone-incorrect-roundings：浮点四舍五入写法错误。

- clang-analyzer（静态路径/数据流）
  - clang-analyzer-core.NullDereference：空指针解引用。
  - clang-analyzer-core.DivideZero：除零。
  - clang-analyzer-core.uninitialized.Assign：未初始化值被使用。
  - clang-analyzer-deadcode.DeadStores：写入的值从未被读取。
  - clang-analyzer-cplusplus.NewDeleteLeaks：new/delete 不匹配或泄漏。
  - clang-analyzer-cplusplus.Move：错误的移动语义导致使用已移走对象。
  - clang-analyzer-core.CallAndMessage：非法调用/消息发送。
  - clang-analyzer-optin.cplusplus.UninitializedObject：对象成员未初始化。
  - clang-analyzer-unix.Malloc：malloc/free 不匹配或泄漏。
  - clang-analyzer-security.insecureAPI.strcpy：不安全 C API 使用。

- modernize（现代化）
  - modernize-use-override：覆盖虚函数时使用 override。
  - modernize-use-nullptr：使用 nullptr 取代 NULL/0。
  - modernize-loop-convert：传统 for 转范围 for。
  - modernize-use-auto：较长类型名时使用 auto（结合阈值）。
  - modernize-use-using：用 using 替代 typedef。
  - modernize-use-default-member-init：成员在声明处默认初始化。
  - modernize-use-equals-default：可默认的特殊成员函数使用 = default。
  - modernize-avoid-c-arrays：避免原生 C 数组，使用 std 容器/视图。
  - modernize-raw-string-literal：使用原始字符串字面量提升可读性。
  - modernize-redundant-void-arg：移除无意义的 `(void)` 形参列表。

- performance（性能）
  - performance-unnecessary-copy-initialization：避免无谓复制初始化。
  - performance-unnecessary-value-param：入参只读时用 const&。
  - performance-for-range-copy：范围 for 避免按值遍历大对象。
  - performance-inefficient-string-concatenation：循环拼接前 reserve。
  - performance-move-const-arg：对 const& 实参 std::move 无效。
  - performance-noexcept-move-constructor：移动构造应标注 noexcept。
  - performance-unnecessary-string-conversions：避免多余的 string 转换。
  - performance-implicit-conversion-in-loop：循环中隐式转换带来额外开销。
  - performance-faster-string-find：使用更高效的字符串查找形式。
  - performance-trivially-destructible：优先使用可平凡析构的类型以利优化。

- readability（可读性）
  - readability-braces-around-statements：单句也加大括号，避免悬挂逻辑。
  - readability-identifier-naming：统一命名风格（类/函数/变量等）。
  - readability-implicit-bool-conversion：显式的布尔转换，避免误解。
  - readability-redundant-declaration：移除冗余声明。
  - readability-container-size-empty：用 `.empty()` 替代 `size()==0`。
  - readability-named-parameter：函数声明为参数命名，提升文档性。
  - readability-inconsistent-declaration-parameter-name：声明/定义参数名保持一致。
  - readability-static-accessed-through-instance：静态成员通过类名访问。
  - readability-simplify-boolean-expr：简化布尔表达式。
  - readability-uniqueptr-delete-release：unique_ptr 删除器使用规范。

- cppcoreguidelines（核心指南）
  - cppcoreguidelines-avoid-magic-numbers：避免魔法数字，使用具名常量。
  - cppcoreguidelines-pro-type-member-init：成员统一初始化。
  - cppcoreguidelines-narrowing-conversions：避免窄化转换丢数据。
  - cppcoreguidelines-owning-memory：所有权明确，优先智能指针/RAII。
  - cppcoreguidelines-pro-bounds-array-to-pointer-decay：数组到指针衰减风险。
  - cppcoreguidelines-slicing：对象切片风险（多态丢失）。
  - cppcoreguidelines-avoid-goto：避免 `goto` 带来的控制流复杂性。
  - cppcoreguidelines-pro-type-reinterpret-cast：限制 reinterpret_cast 的使用。
  - cppcoreguidelines-init-variables：变量声明时初始化。
  - cppcoreguidelines-no-malloc：限制直接 malloc/free，使用 RAII/容器。

- cert（CERT 安全编码）
  - cert-dcl03-c：禁止在头文件定义非内联函数等，避免 ODR/链接问题。
  - cert-err58-cpp：禁止在异常展开时调用非 noexcept 的析构/回调。
  - cert-msc50-cpp：禁止依赖未定义行为的实现细节。
  - cert-msc51-cpp：禁止使用 rand 产生不可预测随机数（建议 <random>）。
  - cert-oop54-cpp：复制控制正确（拷贝/移动/赋值/析构一致）。
  - cert-dcl59-cpp：禁止可移植性差的可变参数宏。
  - cert-env33-c：谨慎使用环境变量，避免安全风险。
  - cert-str34-c：字符串处理安全（边界/终止符）。
  - cert-con36-c：避免隐式转换导致安全问题。
  - cert-flp30-c：浮点比较与精度安全。

- hicpp（High Integrity C++）
  - hicpp-exception-baseclass：自定义异常应继承自 std::exception。
  - hicpp-use-override：同 modernize-use-override，覆盖声明清晰。
  - hicpp-no-assembler：禁止内联汇编。
  - hicpp-avoid-goto：避免 goto（同核心指南）。
  - hicpp-static-assert：使用 static_assert 增强编译期校验。
  - hicpp-vararg：避免 C 可变参数。
  - hicpp-use-auto：在复杂类型上使用 auto（同 modernize）。
  - hicpp-deprecated-headers：避免使用废弃头文件。
  - hicpp-multiway-paths-covered：switch 覆盖所有分支（含 default）。
  - hicpp-explicit-conversions：显式构造与转换。

- portability（可移植性）
  - portability-simd-intrinsics：SIMD 内在函数的可移植性提示。
  - portability-restrict-system-includes：限制系统 include 的差异使用。
  - portability-llvm-include-order：LLVM 风格 include 顺序。
  - portability-unix-signals：跨平台信号使用注意事项。
  - portability-constexpr-math：常量表达式数学函数的可移植差异。
  - portability-header-path：头路径分隔符与大小写敏感问题。

- misc（杂项）
  - misc-definitions-in-headers：禁止在头文件定义非内联函数或变量。
  - misc-unused-parameters：未使用的形参应标注或移除。
  - misc-non-private-member-variables-in-classes：类的非私有成员变量过多。
  - misc-new-delete-overloads：自定义 new/delete 的正确性与一致性。
  - misc-throw-by-value-catch-by-reference：异常按值抛出、按引用捕获。
  - misc-static-assert：充分使用 static_assert 进行约束。
  - misc-no-recursion：递归使用的风险与限制。
  - misc-misplaced-const：const 位置不规范影响可读性。
  - misc-include-cleaner：包含清理相关提示（依赖 include-what-you-use 思路）。

## 15. 推荐阅读与学习路径

- **基础与语法**
  - 《C++ Primer（第5版）》
  - [A Tour of C++（第3版）](https://www.stroustrup.com/Tour.html)
  - [cppreference（权威参考）](https://en.cppreference.com)
- **工程实践与质量**
  - 《高质量 C++/C 编程指南》（林锐）
  - 《Effective C++（第3版）》、 《More Effective C++》、 《Effective STL》
- **现代 C++（C++11/14/17/20）**
  - [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)
  - 《深入理解 C++11：C++11 新特性解析与应用》（Michael Wong 等，IBM XL 编译器团队）
  - 《C++17/20 - The Complete Guide》（Josuttis）
- **标准与指南**
  - [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
  - [Guidelines Support Library (GSL)](https://github.com/microsoft/GSL)
  - [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)
  - [clang-format Style Options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
  - [clang-tidy Checks 索引](https://clang.llvm.org/extra/clang-tidy/checks/list.html)
- **并发与性能（专题）**
  - [C++ Concurrency in Action（第2版）](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)
  - 《High Performance C++》（或 Josuttis 的性能专题材料，按需选读）
- **建议阅读路径（最小集）**
  - 入门：A Tour of C++ → C++ Primer → 熟练检索 cppreference
  - 现代化：Effective Modern C++ → Core Guidelines 精选条目
  - 工程质量：Effective C++/More Effective C++/Effective STL → 高质量 C++/C 编程指南
  - 专题：Concurrency in Action（并发）→ 基于项目选读性能材料
