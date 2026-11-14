#include <iostream>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <stdexcept>
using namespace std;

enum class NodeKind { Number, Variable, Operation };

template <typename T>
class Node {
private:
    NodeKind kind_;
    T value_;
    string name_;
    char op_;
    shared_ptr<Node<T>> left_;
    shared_ptr<Node<T>> right_;

    T calculate(T a, T b) const {
        if (op_ == '+') return a + b;
        if (op_ == '-') return a - b;
        if (op_ == '*') return a * b;
        if (b == T(0)) {
            throw runtime_error("Division by zero");
        }
        return a / b;
    }

public:
    Node(T value) : kind_(NodeKind::Number), value_(value), op_(' ') {}

    Node(const string& name) : kind_(NodeKind::Variable), value_(T()), name_(name), op_(' ') {
        if (name.empty()) {
            throw invalid_argument("Variable name must not be empty");
        }
    }

    Node(char op, shared_ptr<Node<T>> left, shared_ptr<Node<T>> right)
        : kind_(NodeKind::Operation), value_(T()), op_(op), left_(left), right_(right) {
        if (op != '+' && op != '-' && op != '*' && op != '/') {
            throw invalid_argument("Unknown operation");
        }
        if (left == nullptr || right == nullptr) {
            throw invalid_argument("Operation needs two operands");
        }
    }

    bool isNumber() const { return kind_ == NodeKind::Number; }
    T value() const { return value_; }

    T evaluate(const map<string, T>& values) const {
        if (kind_ == NodeKind::Number) {
            return value_;
        }
        if (kind_ == NodeKind::Variable) {
            if (values.count(name_) == 0) {
                throw runtime_error("Unknown variable: " + name_);
            }
            return values.at(name_);
        }
        return calculate(left_->evaluate(values), right_->evaluate(values));
    }

    string toString() const {
        if (kind_ == NodeKind::Number) {
            ostringstream text;
            text << value_;
            return text.str();
        }
        if (kind_ == NodeKind::Variable) {
            return name_;
        }
        return "(" + left_->toString() + " " + op_ + " " + right_->toString() + ")";
    }

    set<string> variables() const {
        set<string> names;
        if (kind_ == NodeKind::Variable) {
            names.insert(name_);
        }
        if (kind_ == NodeKind::Operation) {
            set<string> leftNames = left_->variables();
            set<string> rightNames = right_->variables();
            names.insert(leftNames.begin(), leftNames.end());
            names.insert(rightNames.begin(), rightNames.end());
        }
        return names;
    }

    int depth() const {
        if (kind_ != NodeKind::Operation) {
            return 1;
        }
        int leftDepth = left_->depth();
        int rightDepth = right_->depth();
        if (leftDepth > rightDepth) return 1 + leftDepth;
        else return 1 + rightDepth;
    }

    int nodeCount() const {
        if (kind_ != NodeKind::Operation) {
            return 1;
        }
        return 1 + left_->nodeCount() + right_->nodeCount();
    }

    shared_ptr<Node<T>> simplify() const {
        if (kind_ == NodeKind::Number) {
            return make_shared<Node<T>>(value_);
        }
        if (kind_ == NodeKind::Variable) {
            return make_shared<Node<T>>(name_);
        }
        shared_ptr<Node<T>> left = left_->simplify();
        shared_ptr<Node<T>> right = right_->simplify();
        bool divisionByZero = (op_ == '/' && right->isNumber() && right->value() == T(0));
        if (left->isNumber() && right->isNumber() && !divisionByZero) {
            return make_shared<Node<T>>(calculate(left->value(), right->value()));
        }
        return make_shared<Node<T>>(op_, left, right);
    }
};

class Tester {
private:
    int failed_;
public:
    Tester() : failed_(0) {}

    void check(bool condition, const string& name) {
        if (condition) {
            cout << "[OK]   " << name << '\n';
        }
        else {
            cout << "[FAIL] " << name << '\n';
            failed_++;
        }
    }

    void printResult() const {
        if (failed_ == 0) {
            cout << "All tests passed\n\n";
        }
        else {
            cout << failed_ << " tests FAILED\n\n";
        }
    }
};

int main()
{
    Tester test;

    auto two = make_shared<Node<double>>(2.0);
    auto three = make_shared<Node<double>>(3.0);
    auto four = make_shared<Node<double>>(4.0);
    auto price = make_shared<Node<double>>(string("price"));
    auto count = make_shared<Node<double>>(string("count"));
    auto delivery = make_shared<Node<double>>(string("delivery"));

    auto sum = make_shared<Node<double>>('+', two, three);
    auto onlyConstants = make_shared<Node<double>>('*', sum, four);
    auto oneVariable = make_shared<Node<double>>('*', sum, price);
    auto severalVariables = make_shared<Node<double>>('+',
        make_shared<Node<double>>('*', price, count), delivery);
    auto nested = make_shared<Node<double>>('-', severalVariables,
        make_shared<Node<double>>('/', four, two));

    test.check(onlyConstants->toString() == "((2 + 3) * 4)", "Text of expression");
    test.check(onlyConstants->evaluate({}) == 20, "Only constants: 20");
    test.check(onlyConstants->simplify()->toString() == "20", "Only constants simplify to 20");

    test.check(oneVariable->evaluate({ {"price", 10} }) == 50, "price = 10 gives 50");
    test.check(oneVariable->evaluate({ {"price", 2} }) == 10, "price = 2 gives 10");
    test.check(oneVariable->simplify()->toString() == "(5 * price)", "Simplify (2 + 3) * price");
    test.check(oneVariable->toString() == "((2 + 3) * price)", "Original tree unchanged");

    map<string, double> order = { {"price", 10}, {"count", 3}, {"delivery", 5} };
    test.check(severalVariables->evaluate(order) == 35, "Several variables: 35");
    test.check(severalVariables->variables() == set<string>{"count", "delivery", "price"}, "Variables found");

    test.check(nested->depth() == 4, "Nested depth is 4");
    test.check(nested->nodeCount() == 9, "Nested has 9 nodes");
    test.check(nested->evaluate(order) == 33, "Nested: 35 - 4 / 2 = 33");

    bool unknownCaught = false;
    try {
        severalVariables->evaluate({ {"price", 10} });
    }
    catch (const runtime_error&) {
        unknownCaught = true;
    }
    test.check(unknownCaught, "Unknown variable is reported");

    auto zero = make_shared<Node<double>>(0.0);
    auto divideByZero = make_shared<Node<double>>('/', price, zero);
    bool zeroCaught = false;
    try {
        divideByZero->evaluate({ {"price", 1} });
    }
    catch (const runtime_error&) {
        zeroCaught = true;
    }   
    test.check(zeroCaught, "Division by zero is reported");

    auto seven = make_shared<Node<int>>(7);
    auto twoInt = make_shared<Node<int>>(2);
    auto intDivision = make_shared<Node<int>>('/', seven, twoInt);
    test.check(intDivision->evaluate({}) == 3, "Works with int: 7 / 2 = 3");

    test.printResult();

    cout << "Expression: " << severalVariables->toString() << '\n';
    cout << "price=10 -> " << severalVariables->evaluate(order) << '\n';
    order["price"] = 20;
    cout << "price=20 -> " << severalVariables->evaluate(order) << '\n';
    cout << "Before simplify: " << oneVariable->toString() << '\n';
    cout << "After simplify:  " << oneVariable->simplify()->toString() << '\n';
}