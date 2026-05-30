# Fall Detection v4 (fall-detect-who-v4)

ESP32-P4 人体跌倒检测 v4 版本，基于 esp-who 框架。

## 版本说明

| 项目 | 值 |
|------|-----|
| 模型版本 | **v4** |
| 模型文件 | `espdet_pico_224_224_fall_v4.espdl` |
| 模型大小 | 501 KB |
| 枚举标识 | `ESPDET_PICO_224_224_FALL_V4` |
| 置信度阈值 | 0.45 |
| Kconfig 菜单 | `models: fall_v4_detect` |
| 数据集 | Fall Detection.v3-resized640_aug5x-fast.yolov11 + Negative_sample_dataset |

## 模型详情

| 项目 | 值 |
|------|-----|
| 模型名称 | espdet_pico_224_224_fall_v4 |
| 模型架构 | ESPDet-Pico（单阶段 Anchor-based 检测器）|
| 输入尺寸 | 224 × 224 RGB |
| 检测类别 | 1 类（fall — 跌倒）|
| 预处理 | mean={0,0,0}, std={255,255,255}, letterbox fill={114,114,114} |
| 后处理 | stride {8,16,32}, shape {(4,4),(8,8),(16,16)} |
| 量化 | INT8, esp32p4 target, 32 calib steps, equalization |

## 训练信息（v4）

训练代码：`d:\AAAxiaozhi\esp-detection-main`

### 数据集

| 数据 | 路径 | 大小 |
|------|------|------|
| 正样本 zip | `D:\AAAxiaozhi\dataset\fall-detect\Fall Detection.v3-resized640_aug5x-fast.yolov11.zip` | 849 MB |
| 负样本 zip | `D:\AAAxiaozhi\dataset\fall-detect\Negative_sample_dataset.zip` | 117 MB |

> ⚠️ 数据集超过 GitHub 100MB 限制，不纳入 git。请从本地 `D:\AAAxiaozhi\dataset\fall-detect\` 获取。

```yaml
# datasets/data.yaml
path: D:/AAAxiaozhi/dataset/fall-detect/Fall Detection.v3-resized640_aug5x-fast.yolov11
train: images/train
val: images/val
negative_setting:
  neg_ratio: 0.15
  use_extra_neg: True
  extra_neg_sources:
    "D:/AAAxiaozhi/dataset/fall-detect/Negative_sample_dataset": 654
  fix_dataset_length: 20000
names:
  0: Fall
```

### 模型架构

```yaml
# cfg/models/espdet_pico.yaml
nc: 1
backbone: Conv→DSConv→ESPBlockLite→DSConv→DSC3k2×2→SCDown→DSC3k2×2→SCDown→DSC3k2×2→SPPF→DSConv
head: FPN + ESPDetect(nc=1, reg_max=1), 3 层输出 (P3/P4/P5, stride 8/16/32)
```

### 训练超参数

| 参数 | 值 | 参数 | 值 |
|------|-----|------|-----|
| epochs | 300 | optimizer | AdamW |
| imgsz | 224×224 | lr0 | 0.001 |
| batch | 128 | lrf | 0.01 |
| warmup | 5 epochs | weight_decay | 0.0005 |
| mosaic | 0.8 | close_mosaic | 30 |
| mixup | 0.1 | cos_lr | True |

### ONNX 导出 + 量化

| 参数 | 值 |
|------|-----|
| ONNX opset | 13 |
| simplify | onnxsim |
| 输出 tensor | box0, score0, box1, score1, box2, score2 |
| 量化精度 | INT8 |
| 校准集 | deploy/fall_calib, 32 steps |
| 均衡 | equalization=True, iterations=4 |

### 一键复现

```bash
cd d:\AAAxiaozhi\esp-detection-main
python espdet_run.py \
  --class_name fall_v4 \
  --dataset datasets/data.yaml \
  --size 224 224 \
  --target esp32p4 \
  --calib_data deploy/fall_calib \
  --espdl espdet_pico_224_224_fall_v4.espdl \
  --img espdet.jpg
```

## 硬件要求

ESP32-P4 Function EV Board + SC2336 MIPI-CSI + EK79007 MIPI-DSI + 32MB PSRAM

## 编译

```bash
idf.py -D SDKCONFIG_DEFAULTS=sdkconfig.bsp.esp32_p4_function_ev_board_noglib -D DETECT_MODEL=fall_detect set-target esp32p4
idf.py build
idf.py -p COM15 flash monitor
```

## 文件结构

```
fall-detect-who-v4/
├── CMakeLists.txt
├── main/
│   ├── app_main.cpp                 # 入口 (score_thr=0.45, fps=10)
│   ├── frame_cap_pipeline.cpp       # P4 PPA/MIPI/UVC 管线
│   └── frame_cap_pipeline.hpp
└── components/
    └── fall_detect/
        ├── espdet_detect.hpp/cpp    # ESPDetDetect(ESPDET_PICO_224_224_FALL_V4)
        ├── Kconfig                  # models: fall_v4_detect
        ├── CMakeLists.txt
        └── models/p4/
            └── espdet_pico_224_224_fall_v4.espdl  # 模型 (501KB)
```

## 代码仓库

https://github.com/thefinalisland001/fall-detect-who (分支 `fall_detect_v4`)