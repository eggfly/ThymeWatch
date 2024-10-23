
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


