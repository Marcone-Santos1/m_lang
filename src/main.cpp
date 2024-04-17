#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std; 

enum class TokenType {
    INT,
    FLOAT,
    BOOL,
    STRING,
    IDENTIFIER,
    EQUAL,
    SEMICOLON,
    PRINTLN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PAREN,
    RIGHT_PAREN,
    END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& input) : input(input), position(0) {}

    Token getNextToken() {
        while (position < input.size()) {
            char currentChar = input[position];
            if (isdigit(currentChar) || currentChar == '.') {
                return readNumber();
            } else if (currentChar == '=') {
                position++;
                return {TokenType::EQUAL, "="};
            } else if (currentChar == ';') {
                position++;
                return {TokenType::SEMICOLON, ";"};
            } else if (currentChar == '{') {
                position++;
                return {TokenType::LEFT_BRACE, "{"};
            } else if (currentChar == '}') {
                position++;
                return {TokenType::RIGHT_BRACE, "}"};
            } else if (currentChar == '(') {
                position++;
                return {TokenType::LEFT_PAREN, "("};
            } else if (currentChar == ')') {
                position++;
                return {TokenType::RIGHT_PAREN, ")"};
            } else if (currentChar == '"') {
                return readString();
            } else if (isalpha(currentChar)) {
                return readIdentifier();
            } else if (isspace(currentChar)) {
                skipWhitespace();
                continue;
            } else {
                std::cerr << "Error: Invalid character '" << currentChar << "'" << std::endl;
                exit(1);
            }
        }
        return {TokenType::END, ""};
    }

private:
    Token readNumber() {
        std::string result;
        bool hasDot = false;
        while (position < input.size() && (isdigit(input[position]) || input[position] == '.')) {
            if (input[position] == '.') {
                if (hasDot) {
                    break; // Error: more than one dot in number
                }
                hasDot = true;
            }
            result += input[position];
            position++;
        }
        return {hasDot ? TokenType::FLOAT : TokenType::INT, result};
    }

    Token readIdentifier() {
        std::string result;
        while (position < input.size() && (isalnum(input[position]) || input[position] == '_')) {
            result += input[position];
            position++;
        }
        if (result == "println") {
            return {TokenType::PRINTLN, result};
        } else if (result == "int") {
            return {TokenType::INT, result};
        } else if (result == "float") {
            return {TokenType::FLOAT, result};
        } else if (result == "bool") {
            return {TokenType::BOOL, result};
        } else if (result == "string") {
            return {TokenType::STRING, result};
        } else if (result == "true" || result == "false") {
            return {TokenType::BOOL, result};
        }
        return {TokenType::IDENTIFIER, result};
    }

    Token readString() {
        std::string result;
        position++; // Skip opening quote
        while (position < input.size() && input[position] != '"') {
            if (input[position] == '\\') {
                // Handle escape sequences
                position++;
                if (position >= input.size()) {
                    std::cerr << "Error: Incomplete escape sequence" << std::endl;
                    exit(1);
                }
                switch (input[position]) {
                    case 'n':
                        result += '\n'; // Newline character
                        break;
                    case 't':
                        result += '\t'; // Tab character
                        break;
                    default:
                        result += input[position]; // Other escaped characters
                        break;
                }
            } else {
                result += input[position];
            }
            position++;
        }
        position++; // Skip closing quote
        return {TokenType::STRING, result};
    }

    void skipWhitespace() {
        while (position < input.size() && isspace(input[position])) {
            position++;
        }
    }

    std::string input;
    size_t position;
};

class Interpreter {
public:
    Interpreter(const std::string& input) : lexer(input), currentToken(lexer.getNextToken()) {}

    void parse() {
        match(TokenType::INT);
        match(TokenType::IDENTIFIER);
        match(TokenType::LEFT_PAREN);
        match(TokenType::RIGHT_PAREN);
        if (currentToken.type == TokenType::LEFT_BRACE) {
            match(TokenType::LEFT_BRACE);
            while (currentToken.type != TokenType::RIGHT_BRACE) {
                if (currentToken.type == TokenType::PRINTLN) {
                    match(TokenType::PRINTLN);
                    match(TokenType::LEFT_PAREN);
                    if (currentToken.type == TokenType::STRING) {
                        std::cout << currentToken.value << std::endl;
                        match(TokenType::STRING);
                    } else {
                        std::cout << evaluateExpression() << std::endl;
                    }
                    match(TokenType::RIGHT_PAREN);
                    match(TokenType::SEMICOLON);
                } else {
                    TokenType variableType = currentToken.type;
                    matchAnyType();
                    std::string identifier = currentToken.value;
                    match(TokenType::IDENTIFIER);
                    if (currentToken.type == TokenType::EQUAL) {
                        match(TokenType::EQUAL);
                        std::string value = evaluateExpression();
                        // Check if the value matches the variable type
                        if (variableType == TokenType::INT && !isInteger(value)) {
                            std::cerr << "Error: Cannot assign non-integer value to an integer variable" << std::endl;
                            exit(1);
                        } else if (variableType == TokenType::FLOAT && !isFloat(value)) {
                            std::cerr << "Error: Cannot assign non-float value to a float variable" << std::endl;
                            exit(1);
                        } else if (variableType == TokenType::BOOL && !isBool(value)) {
                            std::cerr << "Error: Cannot assign non-boolean value to a boolean variable" << std::endl;
                            exit(1);
                        }
                        variables[identifier] = value;
                    }
                    match(TokenType::SEMICOLON);
                }
            }
            match(TokenType::RIGHT_BRACE);
        } else {
            match(TokenType::SEMICOLON);
        }
        match(TokenType::END);
    }

private:
    bool isInteger(const std::string& value) {
        for (char c : value) {
            if (!isdigit(c)) {
                return false;
            }
        }
        return true;
    }

    bool isFloat(const std::string& value) {
        bool hasDot = false;
        for (char c : value) {
            if (c == '.') {
                if (hasDot) {
                    return false; // More than one dot in number
                }
                hasDot = true;
            } else if (!isdigit(c)) {
                return false;
            }
        }
        return true;
    }

    bool isBool(const std::string& value) {
        return (value == "true" || value == "false");
    }

    std::string evaluateExpression() {
        std::string expression;
        if (currentToken.type == TokenType::STRING) {
            expression = currentToken.value;
            currentToken = lexer.getNextToken();
        } else if (currentToken.type == TokenType::INT || currentToken.type == TokenType::FLOAT ||
                   currentToken.type == TokenType::BOOL || currentToken.type == TokenType::IDENTIFIER) {
            expression = evaluateVariable();
        } else {
            std::cerr << "Error: Unexpected token '" << currentToken.value << "', expected an expression" << std::endl;
            exit(1);
        }
        return expression;
    }

    std::string evaluateVariable() {
        std::string result;
        if (currentToken.type == TokenType::IDENTIFIER) {
            std::string varName = currentToken.value;
            if (variables.find(varName) != variables.end()) {
                result = variables[varName];
            } else {
                std::cerr << "Error: Variable '" << varName << "' is not defined" << std::endl;
                exit(1);
            }
            currentToken = lexer.getNextToken();
        } else if (currentToken.type == TokenType::INT || currentToken.type == TokenType::FLOAT ||
                   currentToken.type == TokenType::BOOL) {
            result = currentToken.value;
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "Error: Unexpected token '" << currentToken.value << "', expected a variable or literal value" << std::endl;
            exit(1);
        }
        return result;
    }

    void matchAnyType() {
        TokenType expectedType = currentToken.type;
        std::string varName = currentToken.value;

        if (expectedType != TokenType::INT && expectedType != TokenType::FLOAT && 
            expectedType != TokenType::BOOL && expectedType != TokenType::STRING) {
            std::cerr << "Error: Unexpected token '" << currentToken.value << "', expected a variable type" << std::endl;
            exit(1);
        }
        currentToken = lexer.getNextToken();
    }

    void match(TokenType expectedType) {
        if (currentToken.type == expectedType) {
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "Current Token Type: " << tokenTypeToString(currentToken.type) << std::endl;
            std::cerr << "Expected Token Type: " << tokenTypeToString(expectedType) << std::endl;
            std::cerr << "Error: Unexpected token '" << currentToken.value << "', expected '" << tokenTypeToString(expectedType) << "'" << std::endl;
            exit(1);
        }
    }

    std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::INT: return "INT";
            case TokenType::FLOAT: return "FLOAT";
            case TokenType::BOOL: return "BOOL";
            case TokenType::STRING: return "STRING";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::EQUAL: return "EQUAL";
            case TokenType::SEMICOLON: return "SEMICOLON";
            case TokenType::PRINTLN: return "PRINTLN";
            case TokenType::LEFT_BRACE: return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
            case TokenType::LEFT_PAREN: return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
            case TokenType::END: return "END";
        }
        return "UNKNOWN";
    }

    Lexer lexer;
    Token currentToken;
    std::unordered_map<std::string, std::string> variables; // Map to store variable values
};

int main(int argc, char* argv[]) {

    setlocale(LC_ALL,"ptb");

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file." << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string input = buffer.str();

    Interpreter interpreter(input);
    interpreter.parse();
    std::cout << "Program parsed successfully." << std::endl;
    return 0;
}
