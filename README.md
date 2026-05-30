# Fall Detection Example (fall-detect-who)

基于 esp-who 框架的 ESP32-P4 人体跌倒检测示例。

## 硬件要求

| 组件 | 型号 | 说明 |
|------|------|------|
| 芯片 | ESP32-P4 | P4 Function EV Board |
| 摄像头 | SC2336 | MIPI-CSI, 1024x600, 30fps |
| LCD | EK79007 | MIPI-DSI, 1024x600 |
| PSRAM | 16MB Flash + 32MB PSRAM | sdkconfig 已启用 |

## 软件版本

| 依赖 | 版本 | 说明 |
|------|------|------|
| ESP-IDF | v5.5.3 | C:\esp\v5.5.3\esp-idf |
| esp-dl | v3.3.4 | 神经网络推理框架 |
| esp-who | master | 人脸/目标检测应用框架 |
| esp-video | v1.4.1 | V4L2 摄像头驱动 |
| esp32_p4_function_ev_board_noglib | v5.2.0 | BSP 板级支持包 |
| 工具链 | riscv32-esp-elf-gcc 14.2.0 | C:\Espressif\tools\riscv32-esp-elf\ |

## 模型信息

| 项目 | 值 |
|------|-----|
| 模型名称 | espdet_pico_224_224_fall |
| 模型架构 | ESPDet-Pico（单阶段 Anchor-based 检测器）|
| 输入尺寸 | 224 × 224 RGB |
| 检测类别 | 1 类（fall — 跌倒）|
| 模型文件 | `components/fall_detect/models/p4/espdet_pico_224_224_fall.espdl` |
| 模型大小 | 498 KB |

**预处理参数：**
- 归一化：mean={0,0,0}, std={255,255,255}（像素值 ÷ 255）
- Letterbox：启用，填充值 {114,114,114}

**后处理参数：**
- 锚点尺度：stride {8,16,32}, shape {(4,4),(8,8),(16,16)}（3 个检测层）
- 置信度阈值：0.40（[app_main.cpp](main/app_main.cpp) 中 `set_score_thr(0.40f, 0)`）
- NMS IoU 阈值：0.7（模型默认值）
- 最大检测数：10

## 推理管线

```
Camera (SC2336, 1024x600)
    │
    ▼
WhoP4Cam (V4L2, RGB565)
    │
    ▼
WhoFetchNode ──────────► WhoPPAResizeNode (→ 224×224) ──► WhoDetect (ESPDetDetect, 10fps)
    │                                                          │
    ▼                                                          ▼
WhoFrameLCDDisp (1024×600)                          WhoDetectResultLCDDisp (画框)
    │                                                          │
    └──────────────────── LCD 显示 ────────────────────────────┘
```

- **PPA 硬件缩放**：`get_lcd_mipi_csi_ppa_frame_cap_pipeline()`，摄像头帧经 PPA Scale-Rotate-Mirror 硬件块缩放到 224×224 送入模型，LCD 显示直接从原始 FetchNode 取帧
- **相机镜像**：`horizontal_flip=false, vertical_flip=false`，如需要镜像改为 `true`

## 环境配置与编译

### 1. 环境变量

```bash
IDF_PATH=C:\esp\v5.5.3\esp-idf
IDF_EXTRA_ACTIONS_PATH=d:\AAAxiaozhi\esp-who-master\tools
```

VS Code 用户：已在 `.vscode/settings.json` 的 `idf.customExtraVars` 中配置。

### 2. 首次配置

在 ESP-IDF 命令提示符中：

```bash
cd fall-detect-who
idf.py -D SDKCONFIG_DEFAULTS=sdkconfig.bsp.esp32_p4_function_ev_board_noglib -D DETECT_MODEL=fall_detect set-target esp32p4
```

### 3. 编译烧录

```bash
idf.py build
idf.py -p COM15 flash monitor
```

## 可调参数速查

所有参数集中在 [app_main.cpp](main/app_main.cpp)：

| 参数 | 位置 | 当前值 | 说明 |
|------|------|--------|------|
| 置信度阈值 | `model->set_score_thr(X, 0)` | 0.40 | 调高减少假阳性，调低提高召回 |
| NMS IoU | `model->set_nms_thr(X, 0)` | 0.70（默认） | 调高保留更多重叠框 |
| 推理帧率 | `detect_app->set_fps(X)` | 10.0 | 限制推理频率 |
| 管线模式 | `get_lcd_*_pipeline()` | PPA 模式 | 注释行可切换到非 PPA 或 UVC 模式 |
| 水平镜像 | `WhoP4Cam(..., horizontal_flip)` | false | [frame_cap_pipeline.cpp](main/frame_cap_pipeline.cpp) |

## 已知问题与修复记录

### esp-dl v3.3.4 API 兼容性

esp-who 框架使用的 esp-dl 旧 API 在 v3.3.4 中被移除/重命名，已修复以下文件：

| 文件 | 修改 |
|------|------|
| `components/who_peripherals/who_cam/who_cam_define.hpp` | `DL_IMAGE_PIX_TYPE_RGB565` → `RGB565LE`，`get_img_byte_size` → `get_pix_byte_size` |
| `components/who_frame_cap/who_frame_cap_node.cpp` | `get_img_byte_size` → `get_pix_byte_size` |
| `components/who_app/.../who_detect_result_handle.cpp` | `RGB565` → `RGB565LE`，`cvt_pix` 第5参数移除 `caps` |

如果后续更新 esp-dl 或 esp-who，注意检查这些兼容性问题。

### fall_detect 模型组件 bug 修复

| 文件 | 修复 | 说明 |
|------|------|------|
| `espdet_detect.cpp:58` | `CONFIG_FALL_DETECT_...` → `CONFIG_ESPDET_DETECT_...` | SD 卡宏名拼写错误 |
| `Kconfig:4` | `!CAT_DETECT_MODEL_...` → `!ESPDET_DETECT_MODEL_...` | 复制粘贴自 cat_detect |
| `CMakeLists.txt:47` | `add_fall_target(...)` → `add_custom_target(...)` | CMake 指令不存在 |

### bsp_ext.py 白名单

`tools/bsp_ext.py` 已添加 `fall-detect-who`（EXAMPLES）和 `fall_detect`（DETECT_MODELS）。

## 文件结构

```
fall-detect-who/
├── CMakeLists.txt                        # 工程根 CMake
├── partitions.csv                        # Flash 分区表
├── sdkconfig.bsp.esp32_p4_function_ev_board_noglib  # BSP 默认 Kconfig
├── main/
│   ├── CMakeLists.txt                    # 主组件 CMake
│   ├── idf_component.yml                 # 组件依赖声明
│   ├── app_main.cpp                      # 入口，模型初始化 + 参数调优
│   ├── frame_cap_pipeline.hpp            # 帧捕获管线声明
│   └── frame_cap_pipeline.cpp            # P4 MIPI-CSI / PPA / UVC 管线实现
└── components/
    └── fall_detect/                      # 跌倒检测模型组件
        ├── espdet_detect.hpp/cpp         # ESPDetDetect 封装类
        ├── Kconfig                       # 模型位置 menuconfig 选项
        ├── CMakeLists.txt                # 模型打包 + flash 嵌入
        └── models/p4/
            └── espdet_pico_224_224_fall.espdl  # 模型文件 (498KB)
```

## 代码仓库

https://github.com/thefinalisland001/fall-detect-who