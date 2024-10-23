

# 自定义绘图脚本语言规范及 C/C++ 解释器实现（使用 Adafruit GFX）

## 目录

1. [脚本语言规范](#脚本语言规范)
    - [概述](#概述)
    - [词法规则](#词法规则)
    - [语法规则](#语法规则)
    - [内置函数和指令](#内置函数和指令)
    - [示例脚本](#示例脚本)
2. [解释器实现（C/C++）](#解释器实现cc)
    - [总体设计](#总体设计)
    - [关键组件](#关键组件)
    - [示例代码](#示例代码)
    - [运行说明](#运行说明)
3. [总结](#总结)

---

## 脚本语言规范

### 概述

该脚本语言是一种用于绘制图形的简单脚本语言，旨在简化绘图指令的编写。它具有以下特性：

- **简洁的语法**：类似于伪代码，易于学习和使用。
- **变量声明和赋值**：使用 `let` 关键字来声明变量。
- **表达式计算**：支持基本的算术运算和内置数学函数。
- **绘图指令**：提供内置的绘图函数，如 `drawText`、`drawCircle`、`drawLine` 等。
- **注释**：使用 `#` 开头的行为注释。

### 词法规则

**标记类型**：

- **标识符（Identifier）**：以字母、下划线或美元符号开头，后跟字母、数字、下划线或美元符号的序列。例如：`x`, `y1`, `_value`, `$time`.
- **关键字（Keyword）**：`let`, `drawText`, `drawCircle`, `drawLine`, `sin`, `cos`, `tan`, `sqrt`.
- **数字（Number）**：整数或浮点数。例如：`42`, `3.14`.
- **字符串（String）**：由双引号包围的字符序列。例如：`"Hello, World!"`.
- **运算符（Operator）**：`+`, `-`, `*`, `/`, `%`, `=`.
- **分隔符（Delimiter）**：`(`, `)`, `,`.
- **注释（Comment）**：以 `#` 开头，直到行末的内容。

**空白字符**：

- 空格、制表符和换行符将被忽略，除非它们用于分隔标记。

### 语法规则

以下使用巴科斯范式（BNF）定义语法规则：

```
<Program>       ::= { <Statement> }

<Statement>     ::= <VarDeclStmt> | <ExprStmt>

<VarDeclStmt>   ::= 'let' <Identifier> '=' <Expression>

<ExprStmt>      ::= <Expression>

<Expression>    ::= <AdditionExpr>

<AdditionExpr>  ::= <MultiplicationExpr> { ('+' | '-') <MultiplicationExpr> }

<MultiplicationExpr> ::= <UnaryExpr> { ('*' | '/') <UnaryExpr> }

<UnaryExpr>     ::= [ '+' | '-' ] <PrimaryExpr>

<PrimaryExpr>   ::= <Number> | <Identifier> | <String> | <FunctionCall> | '(' <Expression> ')'

<FunctionCall>  ::= (<Identifier>) '(' [ <ArgumentList> ] ')'

<ArgumentList>  ::= <Expression> { ',' <Expression> }
```

### 内置函数和指令

**变量相关**：

- `let`：用于声明变量，例如：`let x = 10`

**数学函数**：

- `sin(angle)`：正弦函数，`angle` 为弧度。
- `cos(angle)`：余弦函数，`angle` 为弧度。
- `tan(angle)`：正切函数，`angle` 为弧度。
- `sqrt(value)`：平方根函数。

**绘图指令**：

- `drawText(x, y, text, color, fontSize)`：在坐标 (`x`, `y`) 处绘制指定颜色和字体大小的文字。
- `drawCircle(x, y, radius, fillColor, strokeColor, strokeWidth)`：绘制圆形。
- `drawLine(x1, y1, x2, y2, color, strokeWidth)`：绘制直线。
- `drawRectangle(x, y, width, height, fillColor, strokeColor, strokeWidth)`：绘制矩形。
- `drawPath(points, color, strokeWidth)`：绘制路径，`points` 为点的数组。

**注意**：

- 颜色以字符串表示，例如：`"#FF0000"` 表示红色。
- 所有角度均以弧度为单位，若需要从角度转换为弧度，可使用公式：`radian = degree * (π / 180)`。

### 示例脚本

```python
# 定义画布中心坐标
let cx = 160
let cy = 160

# 定义表盘半径
let radius = 150

# 获取当前时间（示例值）
let $hour = 10
let $minute = 30

# 计算指针角度（转换为弧度）
let hourAngle = ($hour % 12 + $minute / 60) * 30 * (3.1416 / 180)
let minuteAngle = $minute * 6 * (3.1416 / 180)

# 绘制表盘外圈
drawCircle(cx, cy, radius, "#FFFFFF", "#000000", 2)

# 绘制小时指针
let hourLength = radius * 0.6
let hourX = cx + sin(hourAngle) * hourLength
let hourY = cy - cos(hourAngle) * hourLength
drawLine(cx, cy, hourX, hourY, "#000000", 8)

# 绘制分钟指针
let minuteLength = radius * 0.8
let minuteX = cx + sin(minuteAngle) * minuteLength
let minuteY = cy - cos(minuteAngle) * minuteLength
drawLine(cx, cy, minuteX, minuteY, "#000000", 5)

# 绘制中心点
drawCircle(cx, cy, 5, "#000000", "#000000", 0)
```

---

## 解释器实现（C/C++）

### 总体设计

解释器的主要功能是将脚本代码解析并执行，完成绘图操作。整体设计分为以下几个部分：

1. **词法分析器（Lexer）**：将源代码转换为一系列的词法记号（Tokens）。
2. **语法分析器（Parser）**：根据语法规则，将词法记号序列解析为抽象语法树（AST）。
3. **抽象语法树节点（AST Nodes）**：表示代码的结构和语义。
4. **解释器（Interpreter）**：遍历 AST，执行代码。
5. **绘图接口**：使用 **Adafruit GFX** 库实现绘图功能。

### 关键组件

#### 1. 词法分析器（Lexer）

- **功能**：将源代码拆分为标记序列，供语法分析器使用。
- **实现思路**：
  - 使用有限状态机（FSM）或正则表达式匹配不同类型的标记。
  - 需要识别的标记类型包括：数字、标识符、字符串、关键字、运算符、分隔符、注释等。

#### 2. 语法分析器（Parser）

- **功能**：根据语法规则，将标记序列解析为 AST。
- **实现思路**：
  - 使用递归下降解析器，根据文法规则解析代码。
  - 为每个语法结构（表达式、语句等）定义对应的解析函数。

#### 3. 抽象语法树节点（AST Nodes）

- **功能**：表示代码的结构，包括变量声明、表达式、函数调用等。
- **节点类型**：
  - **表达式节点（Expression Nodes）**：数字、变量、二元运算、函数调用、字符串等。
  - **语句节点（Statement Nodes）**：变量声明、表达式语句等。

#### 4. 解释器（Interpreter）

- **功能**：执行 AST，维护运行环境（变量表），并调用绘图函数。
- **实现思路**：
  - 遍历 AST，根据节点类型执行相应的操作。
  - 对于表达式，计算其值；对于语句，执行其副作用。
  - 维护符号表，存储变量和函数的值。

#### 5. 绘图接口

- **功能**：提供绘图函数的实现，调用 **Adafruit GFX** 库的函数。
- **实现思路**：
  - 封装绘图函数，如 `drawLine()`、`drawCircle()` 等。
  - 在解释器中，当遇到绘图指令时，调用对应的函数。

### 示例代码

下面，我们将提供示例代码，展示如何使用 C++ 实现该解释器。其中，绘图部分使用 **Adafruit GFX** 库。

**注意**：由于 Adafruit GFX 库通常用于嵌入式设备（如 Arduino），因此需要在合适的硬件环境下运行。如果您使用的是桌面环境，可以使用 **Adafruit GFX Library for SDL**，这是一种用于模拟的库。

#### 1. 准备工作

- **安装 Adafruit GFX 库**：根据您的硬件平台，安装适合的 Adafruit GFX 库。
- **配置硬件**：确保您的开发板（如 Arduino）已连接显示设备（如 TFT LCD）。

#### 2. 示例代码

以下是示例代码，包括解释器的主要��件。
```

// main.cpp

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

// 定义屏幕引脚（根据您的硬件修改）
#define TFT_CS     10
#define TFT_RST    9  // 可以是 RST 或 -1
#define TFT_DC     8

// 初始化屏幕对象
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// === 词法分析器 ===

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>
#include <cmath>

enum TokenType {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_KEYWORD,
    TOKEN_COMMENT
};

struct Token {
    TokenType type;
    std::string text;
};

class Lexer {
public:
    Lexer(const std::string& source) : source(source), pos(0) {}

    Token getNextToken() {
        skipWhitespaceAndComments();
        if (pos >= source.length()) {
            return { TOKEN_EOF, "" };
        }

        char current = source[pos];

        // 识别数字
        if (std::isdigit(current) || current == '.') {
            return lexNumber();
        }

        // 识别标识符或关键字
        if (std::isalpha(current) || current == '_' || current == '$') {
            return lexIdentifier();
        }

        // 识别字符串
        if (current == '\"') {
            return lexString();
        }

        // 识别操作符和标点符号
        if (isOperator(current)) {
            Token token = { TOKEN_OPERATOR, std::string(1, current) };
            pos++;
            return token;
        }

        if (isPunctuation(current)) {
            Token token = { TOKEN_PUNCTUATION, std::string(1, current) };
            pos++;
            return token;
        }

        // 未知字符，跳过或报错
        pos++;
        return getNextToken();
    }

private:
    std::string source;
    size_t pos;

    void skipWhitespaceAndComments() {
        while (pos < source.length()) {
            char current = source[pos];
            if (std::isspace(current)) {
                pos++;
            } else if (current == '#') {
                // 跳过注释
                while (pos < source.length() && source[pos] != '\n') {
                    pos++;
                }
            } else {
                break;
            }
        }
    }

    Token lexNumber() {
        size_t start = pos;
        bool hasDot = false;
        while (pos < source.length() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') {
                if (hasDot) break;
                hasDot = true;
            }
            pos++;
        }
        return { TOKEN_NUMBER, source.substr(start, pos - start) };
    }

    Token lexIdentifier() {
        size_t start = pos;
        while (pos < source.length() && (std::isalnum(source[pos]) || source[pos] == '_' || source[pos] == '$')) {
            pos++;
        }
        std::string text = source.substr(start, pos - start);
        if (text == "let" || text == "drawText" || text == "drawCircle" ||
            text == "drawLine" || text == "sin" || text == "cos" ||
            text == "tan" || text == "sqrt") {
            return { TOKEN_KEYWORD, text };
        }
        return { TOKEN_IDENTIFIER, text };
    }

    Token lexString() {
        pos++; // 跳过引号
        size_t start = pos;
        while (pos < source.length() && source[pos] != '\"') {
            pos++;
        }
        std::string text = source.substr(start, pos - start);
        pos++; // 跳过结尾引号
        return { TOKEN_STRING, text };
    }

    bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
               c == '(' || c == ')' || c == ',' || c == '%' || c == '#';
    }

    bool isPunctuation(char c) {
        return c == ';';
    }
};

// === 抽象语法树节点 ===

class ASTNode {
public:
    virtual ~ASTNode() {}
};

class Expression : public ASTNode {
};

class NumberExpr : public Expression {
public:
    double value;
    NumberExpr(double value) : value(value) {}
};

class VariableExpr : public Expression {
public:
    std::string name;
    VariableExpr(const std::string& name) : name(name) {}
};

class BinaryExpr : public Expression {
public:
    std::string op;
    Expression* left;
    Expression* right;
    BinaryExpr(const std::string& op, Expression* left, Expression* right)
        : op(op), left(left), right(right) {}
};

class CallExpr : public Expression {
public:
    std::string callee;
    std::vector<Expression*> args;
    CallExpr(const std::string& callee, const std::vector<Expression*>& args)
        : callee(callee), args(args) {}
};

class StringExpr : public Expression {
public:
    std::string value;
    StringExpr(const std::string& value) : value(value) {}
};

// 语句节点
class Statement : public ASTNode {
};

class VarDeclStmt : public Statement {
public:
    std::string name;
    Expression* expr;
    VarDeclStmt(const std::string& name, Expression* expr) : name(name), expr(expr) {}
};

class ExprStmt : public Statement {
public:
    Expression* expr;
    ExprStmt(Expression* expr) : expr(expr) {}
};

// === 语法分析器 ===

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) {
        nextToken();
    }

    std::vector<Statement*> parse() {
        std::vector<Statement*> statements;
        while (currentToken.type != TOKEN_EOF) {
            statements.push_back(parseStatement());
        }
        return statements;
    }

private:
    Lexer& lexer;
    Token currentToken;

    void nextToken() {
        currentToken = lexer.getNextToken();
    }

    Statement* parseStatement() {
        if (currentToken.type == TOKEN_KEYWORD && currentToken.text == "let") {
            nextToken();
            return parseVariableDeclaration();
        } else {
            return parseExpressionStatement();
        }
    }

    Statement* parseVariableDeclaration() {
        std::string name = currentToken.text;
        nextToken();
        if (currentToken.text != "=") {
            Serial.println("期望 '='");
        }
        nextToken();
        Expression* expr = parseExpression();
        return new VarDeclStmt(name, expr);
    }

    Statement* parseExpressionStatement() {
        Expression* expr = parseExpression();
        return new ExprStmt(expr);
    }

    Expression* parseExpression() {
        return parseAddition();
    }

    Expression* parseAddition() {
        Expression* left = parseMultiplication();
        while (currentToken.text == "+" || currentToken.text == "-") {
            std::string op = currentToken.text;
            nextToken();
            Expression* right = parseMultiplication();
            left = new BinaryExpr(op, left, right);
        }
        return left;
    }

    Expression* parseMultiplication() {
        Expression* left = parseUnary();
        while (currentToken.text == "*" || currentToken.text == "/") {
            std::string op = currentToken.text;
            nextToken();
            Expression* right = parseUnary();
            left = new BinaryExpr(op, left, right);
        }
        return left;
    }

    Expression* parseUnary() {
        if (currentToken.text == "+" || currentToken.text == "-") {
            std::string op = currentToken.text;
            nextToken();
            Expression* expr = parsePrimary();
            return new BinaryExpr(op, new NumberExpr(0), expr);
        } else {
            return parsePrimary();
        }
    }

    Expression* parsePrimary() {
        if (currentToken.type == TOKEN_NUMBER) {
            double value = std::stod(currentToken.text);
            nextToken();
            return new NumberExpr(value);
        } else if (currentToken.type == TOKEN_IDENTIFIER || currentToken.type == TOKEN_KEYWORD) {
            std::string name = currentToken.text;
            nextToken();
            if (currentToken.text == "(") {
                // 函数调用
                nextToken();
                std::vector<Expression*> args;
                if (currentToken.text != ")") {
                    do {
                        args.push_back(parseExpression());
                        if (currentToken.text == ",") {
                            nextToken();
                        } else {
                            break;
                        }
                    } while (true);
                }
                if (currentToken.text != ")") {
                    Serial.println("期望 ')'");
                }
                nextToken();
                return new CallExpr(name, args);
            } else {
                // 变量
                return new VariableExpr(name);
            }
        } else if (currentToken.type == TOKEN_STRING) {
            std::string value = currentToken.text;
            nextToken();
            return new StringExpr(value);
        } else if (currentToken.text == "(") {
            nextToken();
            Expression* expr = parseExpression();
            if (currentToken.text != ")") {
                Serial.println("期望 ')'");
            }
            nextToken();
            return expr;
        } else {
            Serial.println("无法解析的表达式");
            return nullptr;
        }
    }
};

// === 解释器 ===

class Interpreter {
public:
    Interpreter(const std::vector<Statement*>& statements) : statements(statements) {}

    void run() {
        for (Statement* stmt : statements) {
            execute(stmt);
        }
    }

private:
    std::vector<Statement*> statements;
    std::map<std::string, double> variables;

    void execute(Statement* stmt) {
        if (VarDeclStmt* varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
            double value = evaluateExpression(varDecl->expr);
            variables[varDecl->name] = value;
        } else if (ExprStmt* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
            evaluateExpression(exprStmt->expr);
        }
    }

    double evaluateExpression(Expression* expr) {
        if (NumberExpr* numberExpr = dynamic_cast<NumberExpr*>(expr)) {
            return numberExpr->value;
        } else if (VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr)) {
            return getVariableValue(varExpr->name);
        } else if (BinaryExpr* binExpr = dynamic_cast<BinaryExpr*>(expr)) {
            double left = evaluateExpression(binExpr->left);
            double right = evaluateExpression(binExpr->right);
            if (binExpr->op == "+") {
                return left + right;
            } else if (binExpr->op == "-") {
                return left - right;
            } else if (binExpr->op == "*") {
                return left * right;
            } else if (binExpr->op == "/") {
                return left / right;
            }
        } else if (CallExpr* callExpr = dynamic_cast<CallExpr*>(expr)) {
            if (callExpr->callee == "drawLine") {
                if (callExpr->args.size() != 6) {
                    Serial.println("drawLine 需要6个参数");
                    return 0;
                }
                double x1 = evaluateExpression(callExpr->args[0]);
                double y1 = evaluateExpression(callExpr->args[1]);
                double x2 = evaluateExpression(callExpr->args[2]);
                double y2 = evaluateExpression(callExpr->args[3]);
                std::string color = getStringValue(callExpr->args[4]);
                double strokeWidth = evaluateExpression(callExpr->args[5]);
                drawLine(x1, y1, x2, y2, color, strokeWidth);
            } else if (callExpr->callee == "drawCircle") {
                if (callExpr->args.size() != 6) {
                    Serial.println("drawCircle 需要6个参数");
                    return 0;
                }
                double x = evaluateExpression(callExpr->args[0]);
                double y = evaluateExpression(callExpr->args[1]);
                double radius = evaluateExpression(callExpr->args[2]);
                std::string fillColor = getStringValue(callExpr->args[3]);
                std::string strokeColor = getStringValue(callExpr->args[4]);
                double strokeWidth = evaluateExpression(callExpr->args[5]);
                drawCircle(x, y, radius, fillColor, strokeColor, strokeWidth);
            } else if (callExpr->callee == "sin") {
                double value = evaluateExpression(callExpr->args[0]);
                return sin(value);
            } else if (callExpr->callee == "cos") {
                double value = evaluateExpression(callExpr->args[0]);
                return cos(value);
            } else if (callExpr->callee == "tan") {
                double value = evaluateExpression(callExpr->args[0]);
                return tan(value);
            } else if (callExpr->callee == "sqrt") {
                double value = evaluateExpression(callExpr->args[0]);
                return sqrt(value);
            } else {
                Serial.print("未知的函数: ");
                Serial.println(callExpr->callee.c_str());
            }
        } else if (StringExpr* strExpr = dynamic_cast<StringExpr*>(expr)) {
            // 对于字符串表达式，这里返回0，或者根据需要处理
            return 0;
        }
        return 0;
    }

    double getVariableValue(const std::string& name) {
        if (variables.count(name)) {
            return variables[name];
        } else {
            Serial.print("未定义的变量: ");
            Serial.println(name.c_str());
            return 0;
        }
    }

    std::string getStringValue(Expression* expr) {
        if (StringExpr* strExpr = dynamic_cast<StringExpr*>(expr)) {
            return strExpr->value;
        } else {
            Serial.println("期望字符串类型");
            return "";
        }
    }

    // 绘图函数实现
    void drawLine(double x1, double y1, double x2, double y2, const std::string& colorHex, double strokeWidth) {
        uint16_t color = parseColor(colorHex);
        tft.drawLine((int)x1, (int)y1, (int)x2, (int)y2, color);
    }

    void drawCircle(double x, double y, double radius, const std::string& fillColorHex, const std::string& strokeColorHex, double strokeWidth) {
        uint16_t fillColor = parseColor(fillColorHex);
        uint16_t strokeColor = parseColor(strokeColorHex);
        if (fillColorHex != "#FFFFFF") {
            tft.fillCircle((int)x, (int)y, (int)radius, fillColor);
        }
        if (strokeWidth > 0) {
            tft.drawCircle((int)x, (int)y, (int)radius, strokeColor);
        }
    }

    uint16_t parseColor(const std::string& colorHex) {
        if (colorHex.empty() || colorHex[0] != '#' || colorHex.length() != 7) {
            Serial.print("无效的颜色值: ");
            Serial.println(colorHex.c_str());
            return ST77XX_BLACK;
        }
        uint32_t color = (uint32_t)strtol(colorHex.substr(1).c_str(), nullptr, 16);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        return tft.color565(r, g, b);
    }
};

void setup() {
    // 初始化串口
    Serial.begin(9600);

    // 初始化屏幕
    tft.initR(INITR_BLACKTAB);   // 初始化显示（根据您的屏幕类型）
    tft.fillScreen(ST77XX_WHITE); // 清屏为白色

    // 示例脚本
    std::string code = R"(
        # 定义画布中心坐标
        let cx = 80
        let cy = 80

        # 定义表盘半径
        let radius = 70

        # 获取当前时间（示例值）
        let $hour = 10
        let $minute = 30

        # 计算指针角度（转换为弧度）
        let hourAngle = ($hour % 12 + $minute / 60) * 30 * (3.1416 / 180)
        let minuteAngle = $minute * 6 * (3.1416 / 180)

        # 绘制表盘外圈
        drawCircle(cx, cy, radius, "#FFFFFF", "#000000", 2)

        # 绘制小时指针
        let hourLength = radius * 0.6
        let hourX = cx + sin(hourAngle) * hourLength
        let hourY = cy - cos(hourAngle) * hourLength
        drawLine(cx, cy, hourX, hourY, "#000000", 8)

        # 绘制分钟指针
        let minuteLength = radius * 0.8
        let minuteX = cx + sin(minuteAngle) * minuteLength
        let minuteY = cy - cos(minuteAngle) * minuteLength
        drawLine(cx, cy, minuteX, minuteY, "#000000", 5)

        # 绘制中心点
        drawCircle(cx, cy, 3, "#000000", "#000000", 0)
    )";

    Lexer lexer(code);
    Parser parser(lexer);
    auto statements = parser.parse();

    Interpreter interpreter(statements);
    interpreter.run();
}

void loop() {
    // 主循环不需要执行任何操作
}


```
#### 3. 代码说明

- **设置部分**：
  - 包含 **Arduino** 和 **Adafruit GFX** 库的头文件。
  - 初始化屏幕并设置屏幕引脚（根据您的硬件修改）。

- **Lexer**、**Parser**、**AST 节点**、**Interpreter** 类：
  - 与前面的设计一致，实现了词法分析器、语法分析器、抽象语法树和解释器。

- **绘图函数的实现**：
  - 使用 `tft.drawLine()` 绘制直线。
  - 使用 `tft.drawCircle()` 和 `tft.fillCircle()` 绘制圆形。
  - `parseColor()` 函数用于解析颜色字符串，将十六进制颜色代码转换为 Adafruit GFX 可用的颜色格式。

- **`setup()` 函数**：
  - 初始化串口通信和屏幕。
  - 包含示例脚本代码。
  - 解析并执行脚本。

- **`loop()` 函数**：
  - 由于脚本执行一次即可，这里不需要在 `loop()` 中执行任何操作。

#### 4. 注意事项

- **硬件平台**：该代码适用于 Arduino 等嵌入式平台，需连接支持 **Adafruit GFX** 的显示屏，如 TFT 屏幕。

- **调试输出**：使用 `Serial` 进行调试信息的输出，确保串口通信已初始化。

- **修改引脚配置**：根据您的硬件，调整屏幕引脚定义。

### 运行说明

1. **硬件连接**：

   - 将显示屏连接到 Arduino 或其他微控制器，按照 **Adafruit GFX** 库的要求连接相应的引脚。

2. **软件配置**：

   - 在 Arduino IDE 中安装 **Adafruit GFX Library** 和具体屏幕的硬件库（如 Adafruit ST7735）。

3. **编译和上传**：

   - 将示例代码复制到 Arduino IDE 或其他开发环境中。
   - 确保选择了正确的开发板和串口。
   - 编译并将程序上传到开发板。

4. **查看结果**：

   - 程序运行后，显示屏上将绘制一个手表表盘形状，包含时针和分针。

---

## 总结

通过上述步骤，我们实现了一个简单的脚本解释器，能够解析并执行自定义的绘图脚本。该解释器使用 C++ 编写，绘图部分使用 **Adafruit GFX** 库，在嵌入式平台上运行。

- **脚本语言特性**：
  - 支持变量声明和赋值、表达式计算、内置数学函数和绘图指令。

- **解释器结构**：
  - 包括词法分析器、语法分析器、抽象语法树、解释器和绘图接口。

- **硬件平台**：
  - 适用于 Arduino 等嵌入式系统，需要连接支持 **Adafruit GFX** 的显示屏。

- **扩展方向**：
  - 增加对条件语句、循环等控制结构的支持。
  - 增加更多的内置函数和绘图指令。
  - 改进错误处理，提供更详细的错误信息。
