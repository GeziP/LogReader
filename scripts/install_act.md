# 使用act在本地模拟GitHub Actions

## 什么是act？
`act` 是一个命令行工具，可以在本地运行GitHub Actions工作流，完全模拟CI/CD环境。

## 安装act

### Windows (推荐)
```powershell
# 使用Chocolatey安装
choco install act-cli

# 或使用Scoop安装
scoop install act

# 或手动下载
# 下载地址：https://github.com/nektos/act/releases
```

### 验证安装
```bash
act --version
```

## 本地验证CI/CD流程

### 1. 验证主CI流程
```bash
# 模拟push到main分支触发的CI
act push -W .github/workflows/ci.yml

# 模拟pull request触发的CI
act pull_request -W .github/workflows/ci.yml
```

### 2. 验证代码质量检查
```bash
# 运行代码质量检查workflow
act push -W .github/workflows/code-quality.yml
```

### 3. 验证发布流程（需要tag）
```bash
# 模拟版本标签触发的发布流程
act push -W .github/workflows/release.yml -e .github/events/tag.json
```

### 4. 验证文档生成
```bash
# 运行文档生成workflow
act push -W .github/workflows/docs.yml
```

## 创建测试事件文件

创建 `.github/events/tag.json`：
```json
{
  "ref": "refs/tags/v1.0.0",
  "repository": {
    "name": "LogReader",
    "full_name": "your-username/LogReader"
  }
}
```

## act配置文件 (.actrc)

创建 `.actrc` 文件优化性能：
```
--container-architecture linux/amd64
--artifact-server-path /tmp/artifacts
--env-file .env.local
```

## 常用命令

```bash
# 列出所有可用的workflows
act -l

# 运行特定job
act -j build-windows

# 使用特定Docker镜像
act --container-architecture linux/amd64

# 干运行（查看执行计划）
act -n

# 详细输出
act -v
```

## 注意事项

1. **Docker要求**：act需要Docker运行
2. **资源消耗**：完整的CI流程会消耗较多资源
3. **网络依赖**：某些步骤需要网络连接下载依赖
4. **Windows限制**：某些Linux特定的命令可能无法在Windows容器中运行

## 快速验证脚本

创建 `run_local_ci.bat`：
```batch
@echo off
echo 运行本地CI验证...

echo 1. 检查代码格式...
act push -j format-check -W .github/workflows/ci.yml

echo 2. 检查翻译文件...
act push -j translation-check -W .github/workflows/ci.yml

echo 3. 运行构建...
act push -j build -W .github/workflows/ci.yml

echo 本地CI验证完成！
``` 