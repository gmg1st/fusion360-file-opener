# Fusion 360 文件缩略图提供程序

让 Windows 资源管理器直接显示 Fusion 360 `.f3d` / `.f3z` 文件的 3D 模型预览缩略图，并支持双击直接打开这些文件。

![功能示意](https://img.shields.io/badge/平台-Windows_10/11-blue) ![语言](https://img.shields.io/badge/语言-纯_C-informational) ![架构](https://img.shields.io/badge/架构-x64-success)

## 功能特性

- **缩略图预览**：在资源管理器中直接显示 `.f3d` / `.f3z` 文件内嵌的 3D 模型预览图，无需打开 Fusion 360
- **双击打开**：双击 CAD 文件即可启动并加载到 Fusion 360，支持 `.f3d`、`.f3z`、`.step`、`.stp`、`.iges`、`.igs`、`.sat`、`.smt` 格式
- **纯原生实现**：缩略图提供程序为纯 C 编写的 COM 组件，无 .NET 运行时依赖，加载快、兼容性好
- **Fusion 360 加载项**：提供 Python 加载项，增强 Fusion 360 与系统的文件关联体验

## 工作原理

### 缩略图提取流程

```
.f3d 文件 (本质是 ZIP 容器)
   │
   ├─ 解析 ZIP 条目 → 定位 FusionAssetName[Active]/Previews/small.png
   │
   ├─ 预览图压缩方式判断
   │     ├─ Zstandard (method 93) → 调用 libzstd.dll 解压
   │     └─ Deflate (method 8)   → 调用 zlib1.dll 原始解压 (windowBits=-15)
   │
   ├─ 校验 PNG 签名 (89 50 4E 47)
   │
   └─ GDI+ 高质量缩放 → 返回 HBITMAP 给资源管理器 (WTSAT_RGB)
```

### 关键技术点

| 技术点 | 说明 |
|--------|------|
| COM 组件 | 实现 `IThumbnailProvider` + `IInitializeWithStream` 接口，支持进程外调用 |
| 位数要求 | 必须编译为 **64 位 (x64)** DLL，才能被 64 位资源管理器加载 |
| 解压库 | `libzstd.dll`（Zstandard）+ `zlib1.dll`（Deflate），运行时动态加载 |
| 图像处理 | 动态加载 GDI+，`HighQualityBicubic` 插值缩放，白色背景合成透明区域 |
| 文件关联 | 通过 `F3DLauncher.exe` 包装启动，绕过 Windows UserChoice 哈希保护 |

## 文件结构

```
├── src/                                    # 源代码
│   ├── F3DThumbnailProviderNative.c        # 缩略图提供程序核心（纯 C COM 组件）
│   ├── F3DThumbnailProviderNative.def      # DLL 导出定义
│   ├── test_com.c                          # COM 接口独立测试程序
│   ├── F3DLauncher.cs                      # 文件启动器源码（C#）
│   ├── F3DFileOpener.py                    # Fusion 360 加载项
│   └── F3DFileOpener.manifest              # 加载项清单
├── scripts/                                # 部署与维护脚本
│   ├── register_native_thumbnail.ps1       # HKCU 注册脚本
│   ├── register_hklm.ps1                   # HKLM 注册脚本（需管理员）
│   └── fix_thumbnail_reg.ps1               # 注册表修复脚本
├── open_f3d_launcher.bat                   # 双击打开入口（批处理）
├── open_f3d.vbs                            # 双击打开入口（VBScript，无黑窗口）
├── register_thumbnail.reg                  # 文件关联注册表
└── fusion360_icon.ico                      # 文件类型图标
```

## 安装方法

> 以下以 64 位 Windows 10/11 为例。`<LocalAppData>` 指当前用户目录，即 `C:\Users\<用户名>\AppData\Local`。

### 1. 部署文件

将以下文件放到 `%LocalAppData%\F3DThumbnailProvider\` 目录：

| 文件 | 作用 |
|------|------|
| `F3DThumbnailProviderNative.dll` | 缩略图提供程序主 DLL（64 位） |
| `libzstd.dll` | Zstandard 解压库（64 位） |
| `zlib1.dll` | Deflate 解压库（64 位） |

### 2. 注册 COM 组件

以管理员身份运行 PowerShell，执行：

```powershell
.\scripts\register_hklm.ps1
```

注册内容包括：

- `HKCR\.f3d\ShellEx\{E357FCCD-A995-4576-B01F-234630154E96}` → 指向缩略图提供程序 CLSID
- `HKCR\CLSID\{C7A3F5D2-8E4B-4F6A-9D1E-3B5C7F8A2E6D}\InprocServer32` → 指向 DLL 路径，ThreadingModel = Both

### 3. 刷新缩略图缓存

注册后必须清除旧缓存并重启资源管理器才能生效：

```powershell
# 关闭资源管理器
Stop-Process -Name explorer -Force

# 删除缩略图缓存
Remove-Item "$env:LocalAppData\Microsoft\Windows\Explorer\thumbcache_*.db" -Force

# 重启资源管理器
Start-Process explorer.exe
```

### 4. 安装文件关联（可选）

双击 `register_thumbnail.reg` 导入文件关联，使双击 CAD 文件时通过 `F3DLauncher.exe` 启动。

### 5. 安装 Fusion 360 加载项（可选）

1. 复制 `src/F3DFileOpener.py` 和 `src/F3DFileOpener.manifest` 到
   `%AppData%\Autodesk\Autodesk Fusion 360\API\AddIns\F3DFileOpener\`
2. 在 Fusion 360 中打开 **工具 → 修改 → 插件（Scripts and Add-Ins）**
3. 切换到 **加载项（Add-Ins）** 选项卡，选中 `F3DFileOpener` 并启用

## 编译源码

缩略图提供程序使用 [TCC (Tiny C Compiler)](http://download.savannah.gnu.org/releases/tinycc/) 编译，无重量级工具链依赖：

```powershell
tcc -shared -o F3DThumbnailProviderNative.dll `
    src\F3DThumbnailProviderNative.c `
    src\F3DThumbnailProviderNative.def `
    -lgdiplus -lole32 -luser32 -lkernel32 -lmsvcrt
```

> 编译产物必须为 x64。TCC 默认按当前系统位数生成，64 位系统直接编译即可。

## 故障排除

| 现象 | 可能原因与解决方法 |
|------|--------------------|
| 只显示图标没有缩略图 | ① 缩略图缓存未清——删除 `thumbcache_*.db` 后重启 Explorer；② DLL 位数为 32 位——重新以 x64 编译；③ 注册表 GUID 不对——确认 ShellEx 键值为 `{E357FCCD-A995-4576-B01F-234630154E96}` |
| 缩略图全黑 | 预览图透明区域未合成背景色，检查 `GdipCreateHBITMAPFromBitmap` 背景色参数应为 `0xFFFFFFFF` |
| 部分文件有缩略图部分没有 | 老版本 Fusion 360 保存的文件预览图可能为 Zstd 压缩，确认 `libzstd.dll` 已部署 |
| 缓存"顽固"不刷新 | Explorer 会缓存失败结果。停止 Explorer 进程后再删缓存文件，避免文件被占用 |
| 双击没反应 | 检查 `register_thumbnail.reg` 中 F3DLauncher 路径是否与实际一致 |

## 常见问题

**Q：为什么不用 .NET 写缩略图组件？**
A：实测 Windows 外壳对 .NET COM 组件的加载不可靠，且 32 位 DLL 无法被 64 位 Explorer 加载。纯 C 原生 DLL 直接以 `InprocServer32` 方式注册，兼容性和稳定性最好。

**Q：为什么用 zlib 而不用 Windows 自带解压 API？**
A：`RtlDecompressBuffer` 的 Deflate 模式（15）在很多系统上返回 `STATUS_NOT_SUPPORTED`，Windows 8+ 的 `CreateDecompressor` 也并非所有机器可用。zlib 的原始 Deflate 模式（`windowBits=-15`）跨版本稳定。

**Q：会修改我的 Fusion 360 安装吗？**
A：不会。所有文件部署在用户目录，卸载只需删除 DLL 目录并移除注册表项。

## 许可证

本项目仅供学习与个人使用。Fusion 360 及相关格式版权归 Autodesk 所有。
