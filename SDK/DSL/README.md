# Python SDK and WatchFace SDK

# 自定义表盘设计语言规范

本规范定义了一种基于 JSON 的语言，用于为 176x176 彩屏手表设计自定义表盘。该语言允许您指定字体、自定义年月日、时分秒的位置、大小和字体。如果有指针，支持基本的 2D 矢量图形绘制指令，类似于 Lottie 的设计，但不需要动画（每分钟刷新一帧）。

## JSON 结构概览

表盘配置由一个 JSON 对象定义，具有以下结构：

```json
{
  "background": {
    "image": "path/to/background.png",
    "color": "#FFFFFF"
  },
  "elements": [
    {
      "type": "text",
      "content": "YYYY",
      "font": "Arial",
      "size": 18,
      "color": "#000000",
      "position": { "x": 88, "y": 20 },
      "alignment": "center"
    },
    {
      "type": "hand",
      "hand_type": "hour",
      "length": 50,
      "width": 4,
      "color": "#000000",
      "origin": { "x": 88, "y": 88 }
    }
    // 更多元素...
  ]
}
```

## 主要组件

### background（背景）

- **image**：*可选*，背景图片的路径。
- **color**：*可选*，背景颜色，十六进制表示。

### elements（元素）

元素数组，用于定义表盘上的所有元素。每个元素可以是 **text**（文本）或 **hand**（指针）类型。

#### 文本元素

- **type**：`"text"`
- **content**：显示的文本内容，支持占位符：
  - `YYYY`：年
  - `MM`：月
  - `DD`：日
  - `hh`：时
  - `mm`：分
  - `ss`：秒
- **font**：字体名称。
- **size**：字体大小。
- **color**：文本颜色，十六进制表示。
- **position**：`{ "x": [整数], "y": [整数] }`，元素在屏幕上的位置。
- **alignment**：`"left"`、`"center"` 或 `"right"`。

#### 指针元素

- **type**：`"hand"`
- **hand_type**：`"hour"`、`"minute"` 或 `"second"`。
- **length**：指针长度，像素为单位。
- **width**：指针宽度，像素为单位。
- **color**：指针颜色，十六进制表示。
- **origin**：`{ "x": [整数], "y": [整数] }`，指针旋转的中心点。

## 示例

一个示例配置：

```json
{
  "background": {
    "color": "#FFFFFF"
  },
  "elements": [
    {
      "type": "text",
      "content": "hh:mm",
      "font": "Arial",
      "size": 24,
      "color": "#000000",
      "position": { "x": 88, "y": 140 },
      "alignment": "center"
    },
    {
      "type": "hand",
      "hand_type": "hour",
      "length": 40,
      "width": 4,
      "color": "#FF0000",
      "origin": { "x": 88, "y": 88 }
    },
    {
      "type": "hand",
      "hand_type": "minute",
      "length": 60,
      "width": 2,
      "color": "#00FF00",
      "origin": { "x": 88, "y": 88 }
    },
    {
      "type": "hand",
      "hand_type": "second",
      "length": 70,
      "width": 1,
      "color": "#0000FF",
      "origin": { "x": 88, "y": 88 }
    }
  ]
}
```

## 注意事项

- 所有位置坐标均相对于屏幕的左上角。
- 所有尺寸均以像素为单位。
- 颜色使用十六进制格式（例如，`#FF0000` 表示红色）。
- 字体文件必须在设备上可用或作为资源嵌入。



#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <json/json.h> // 使用 JSON 解析库，如 JsonCpp

// 定义位置结构体
struct Position {
    int x;
    int y;
};

// 定义元素基类
struct Element {
    std::string type;
    Position position;
};

// 定义文本元素
struct TextElement : public Element {
    std::string content;
    std::string font;
    int size;
    std::string color;
    std::string alignment;
};

// 定义指针元素
struct HandElement : public Element {
    std::string hand_type;
    int length;
    int width;
    std::string color;
    Position origin;
};

// 定义表盘结构体
struct WatchFace {
    std::string background_image;
    std::string background_color;
    std::vector<Element*> elements;
};

// 解析表盘配置
WatchFace parseWatchFaceConfig(const std::string& jsonFilePath) {
    std::ifstream configFile(jsonFilePath);
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    Json::parseFromStream(readerBuilder, configFile, &root, &errs);

    WatchFace watchFace;
    if (!root["background"]["image"].isNull()) {
        watchFace.background_image = root["background"]["image"].asString();
    }
    if (!root["background"]["color"].isNull()) {
        watchFace.background_color = root["background"]["color"].asString();
    }

    const Json::Value elements = root["elements"];
    for (const auto& elem : elements) {
        std::string type = elem["type"].asString();
        if (type == "text") {
            TextElement* textElem = new TextElement();
            textElem->type = type;
            textElem->position.x = elem["position"]["x"].asInt();
            textElem->position.y = elem["position"]["y"].asInt();
            textElem->content = elem["content"].asString();
            textElem->font = elem["font"].asString();
            textElem->size = elem["size"].asInt();
            textElem->color = elem["color"].asString();
            textElem->alignment = elem["alignment"].asString();
            watchFace.elements.push_back(textElem);
        } else if (type == "hand") {
            HandElement* handElem = new HandElement();
            handElem->type = type;
            handElem->position.x = elem["position"]["x"].asInt();
            handElem->position.y = elem["position"]["y"].asInt();
            handElem->hand_type = elem["hand_type"].asString();
            handElem->length = elem["length"].asInt();
            handElem->width = elem["width"].asInt();
            handElem->color = elem["color"].asString();
            handElem->origin.x = elem["origin"]["x"].asInt();
            handElem->origin.y = elem["origin"]["y"].asInt();
            watchFace.elements.push_back(handElem);
        }
    }

    return watchFace;
}

// 渲染表盘
void renderWatchFace(const WatchFace& watchFace) {
    // 实现背景渲染逻辑
    if (!watchFace.background_image.empty()) {
        // 加载并绘制背景图片
    } else if (!watchFace.background_color.empty()) {
        // 绘制背景颜色
    }

    // 获取当前时间
    std::time_t t = std::time(nullptr);
    std::tm currentTime = *std::localtime(&t);

    // 渲染元素
    for (const auto& elem : watchFace.elements) {
        if (elem->type == "text") {
            TextElement* textElem = dynamic_cast<TextElement*>(elem);
            // 处理占位符替换
            std::string content = textElem->content;
            // 替换年份
            size_t pos = content.find("YYYY");
            if (pos != std::string::npos) {
                content.replace(pos, 4, std::to_string(1900 + currentTime.tm_year));
            }
            // 类似地替换 MM、DD、hh、mm、ss

            // 绘制文本（实现绘制函数）
            // drawText(content, textElem->position, textElem->font, textElem->size, textElem->color, textElem->alignment);
        } else if (elem->type == "hand") {
            HandElement* handElem = dynamic_cast<HandElement*>(elem);
            // 根据 hand_type 和当前时间计算角度
            double angle = 0.0;
            if (handElem->hand_type == "hour") {
                angle = (currentTime.tm_hour % 12 + currentTime.tm_min / 60.0) * 30.0;
            } else if (handElem->hand_type == "minute") {
                angle = (currentTime.tm_min + currentTime.tm_sec / 60.0) * 6.0;
            } else if (handElem->hand_type == "second") {
                angle = currentTime.tm_sec * 6.0;
            }
            // 绘制指针（实现绘制函数）
            // drawHand(handElem->origin, handElem->length, handElem->width, angle, handElem->color);
        }
    }
}

// 主函数
int main() {
    WatchFace watchFace = parseWatchFaceConfig("watchface.json");
    renderWatchFace(watchFace);
    // 清理内存
    for (auto elem : watchFace.elements) {
        delete elem;
    }
    return 0;
}
```

**注意：**

- 该代码使用 [JsonCpp](https://github.com/open-source-parsers/jsoncpp) 库进行 JSON 解析。
- `drawText` 和 `drawHand` 等函数需要根据您的图形框架和硬件自行实现。
- 在渲染文本时，需要替换内容中的时间占位符，如 `YYYY`、`MM`、`DD`、`hh`、`mm`、`ss`。
- 请确保在实际实现中处理资源管理、错误检查和边缘情况。
