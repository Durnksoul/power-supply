# 字体生成工具

本机全局 Python 3.12.10 搭配 Pillow 9.5.0 不在官方兼容范围内，曾在中文字体生成时触发原生崩溃。本工程使用独立环境，避免修改其他软件依赖。

在工程根目录执行：

```powershell
python -m venv build/font-tools
build/font-tools/Scripts/python.exe -m pip install --only-binary=:all: Pillow==12.3.0
build/font-tools/Scripts/python.exe tools/generate_ui_font.py C:/Windows/Fonts/simhei.ttf
```

生成 `Core/Src/ui_font_zh_16.c`。普通固件构建直接使用此文件，不需要运行 Python。
脚本仅生成页面使用的汉字，ASCII 使用 LVGL 字体回退。字体由调用者提供，当前使用本机黑体；分发字体资源时遵循源字体许可。
