#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
using namespace std;




class Vector2D {
private:
    int dx_, dy_;
public:
    Vector2D(int dx, int dy) : dx_(dx), dy_(dy) {}
    int dx() const {
        return dx_;
    }
    int dy() const {
        return dy_;
    }
    bool operator==(const Vector2D& other) const {
        if (dx_ == other.dx() && dy_ == other.dy()) {
            return true;
        }
        else return false;
    }
    bool operator!=(const Vector2D& other) const {
        if (*this == other) {
            return false;
        }
        else return true;
    }
    Vector2D operator+(const Vector2D & other) const{
        int xx = dx_ + other.dx();
        int yy = dy_ + other.dy();
        Vector2D obj(xx, yy);
        return obj;
    }
    Vector2D operator-(const Vector2D& other) const {
        int xx = dx_ - other.dx();
        int yy = dy_ - other.dy();
        Vector2D obj(xx, yy);
        return obj;
    }
};



class Point2D {
private:
    int x_, y_;
public:
    Point2D (int x, int y): x_(x), y_(y) {}
    int x() const {
        return x_;
    }
    int y() const {
        return y_;
    }
    bool operator==(const Point2D & other ) const {
        if (x_ ==other.x() && y_ == other.y()) {
            return true;
        }
        else return false;
    }
    bool operator!=(const Point2D& other) const {
        if (*this == other) {
            return false;
        }
        else return true;
    }
    Point2D operator+(const Vector2D& shift) const {
        int newX = x_ + shift.dx();
        int newY = y_ + shift.dy();
        Point2D obj(newX, newY);
        return obj;
    }
    Vector2D operator-(const Point2D& other) const {
        int dx = x_ - other.x();
        int dy = y_ - other.y();
        Vector2D obj(dx, dy);
        return obj;
    }
};

class Rect {
private:
    Point2D topleft_;
    int width_;
    int height_;
public:
    Rect(const Point2D & topleft, int width, int height) : topleft_(topleft), width_(width), height_(height) { if (width <= 0 || height <= 0) throw invalid_argument("height and width must be > 0"); }
    Point2D topleft() const { return topleft_; }
    int width() const { return width_; }
    int height() const { return height_; }
    bool contains(const Point2D& obj) const {
        if (obj.x() >= topleft_.x() && obj.x() < topleft_.x() + width_ &&
            obj.y() >= topleft_.y() && obj.y() < topleft_.y() + height_) {
            return true;
        }
        else return false;
    }
    bool operator==(const Rect& other) const {
        if (topleft_ == other.topleft() && width_ == other.width() && height_ == other.height()) {
            return true;
        }
        else return false;
    }
    bool operator!=(const Rect& other) const {
        if (*this == other) {
            return false;
        }
        else return true;
    }
};


class Element {
private:
    Rect area_;
    bool enable_;
public:
    Element(const Rect& area) : area_(area), enable_(true) {}
    Rect area() const { return area_; }
    bool isEnabled() const { return enable_; }
    void setEnable(bool enabled) {enable_ = enabled; }
    bool contains(const Point2D & tochka) const{ return area_.contains(tochka); }
    virtual void onClick() = 0;
    virtual string toString() const = 0; // вернет баттон: (и колво потом) 
    virtual bool canFocus() const { return false; } //можно ли вообще делать это целью для ввода текста ?
    virtual void onTextInput(const string& text) {} // сохранил текст дополнительный который потом досиываешь 
    virtual ~Element() {}
};


class Button : public Element {
private:
    int clicks_;
public:
    Button(const Rect& area) : Element(area), clicks_(0) {}

    int clicks() const { return clicks_; }

    void onClick() override {
        if (isEnabled()) {
            clicks_++;
        }
    }

    string toString() const override {
        return "Button:" + to_string(clicks_);
    }
};


class CheckBox : public Element {
private:
    bool checked_;
public:
    CheckBox(const Rect& area) : Element(area), checked_(false) {}

    bool isChecked() const { return checked_; }

    void onClick() override {
        if (isEnabled()) {
            checked_ = !checked_;
        }
    }

    string toString() const override {
        if (checked_) {
            return "[x] CheckBox";
        }
        else return "[ ] CheckBox";
    }
};


class TextField: public Element {
private:
    string text_;
public:
    TextField(const Rect & area) :  Element (area), text_("") {}
    string text() const { return text_; }
    void onClick() override {}
    string toString()const override { if (text_ != "") return text_; else return "TextField"; }
    bool canFocus() const override { if (isEnabled()) return true; else return false; }
    void onTextInput(const string& text) override { if (isEnabled()) text_ += text; }
};


class Form {
private:
    vector<unique_ptr<Element>> elements_;
    int focused_;

    bool isValidIndex(int index) const {
        if (index >= 0 && index < (int)elements_.size()) {
            return true;
        }
        else return false;
    }
    void putChar(vector<string>& canvas, int x, int y, char symbol) const {
        if (y >= 0 && y < (int)canvas.size() && x >= 0 && x < (int)canvas[y].size()) {
            canvas[y][x] = symbol;
        }
    }
public:
    Form() : focused_(-1) {}

    void add(unique_ptr<Element> element) {
        elements_.push_back(move(element));
    }// добавляем элемент в вектор 

    void remove(int index) {
        if (!isValidIndex(index)) {
            return;
        }
        elements_.erase(elements_.begin() + index);
        if (focused_ == index) {
            focused_ = -1;
        }
        else if (focused_ > index) {
            focused_--;
        }
    }

    void setEnabled(int index, bool enabled) {
        if (!isValidIndex(index)) {
            return;
        }
        elements_[index]->setEnable(enabled);
        if (!enabled && focused_ == index) {
            focused_ = -1;
        }
    }

    int elementAt(const Point2D& point) const {
        for (int i = elements_.size() - 1; i >= 0; i--) {
            if (elements_[i]->contains(point)) {
                return i;
            }
        }
        return -1;
    }

    void click(const Point2D& point) {
        int i = elementAt(point);
        if (i != -1) {
            if (elements_[i]->isEnabled()) {
                elements_[i]->onClick();
                if (elements_[i]->canFocus()) {
                    focused_ = i;
                }
            }
        }
    }

    void typeText(const string& text) {
        if (focused_ != -1) {
            elements_[focused_]->onTextInput(text);
        }
    }

    void focusNext() {
        if (elements_.empty()) {
            return;
        }
        int size = elements_.size();
        for (int step = 1; step <= size; step++) {
            int index = (focused_ + step) % size;
            if (elements_[index]->canFocus()) {
                focused_ = index;
                return;
            }
        }
    }

    int focusedIndex() const { return focused_; }

    int count() const { return elements_.size(); }

    string elementText(int index) const {
        if (!isValidIndex(index)) {
            return "";
        }
        return elements_[index]->toString();
    }
    void draw(int width, int height) const {
        vector<string> canvas(height, string(width, ' '));

        for (int i = 0; i < (int)elements_.size(); i++) {
            Rect area = elements_[i]->area();
            int left = area.topleft().x();
            int top = area.topleft().y();
            int right = left + area.width() - 1;
            int bottom = top + area.height() - 1;

            for (int y = top; y <= bottom; y++) {
                for (int x = left; x <= right; x++) {
                    char symbol = ' ';
                    if (y == top || y == bottom) {
                        symbol = '-';
                    }
                    else if (x == left || x == right) {
                        symbol = '|';
                    }
                    if (i == focused_ && symbol != ' ') {
                        symbol = '#';
                    }
                    putChar(canvas, x, y, symbol); // за рамку не пишет свою палку? проерить не забыть !!!!!
                }
            }

            string text = elements_[i]->toString();
            for (int k = 0; k < (int)text.size() && left + 1 + k < right/* на рамку правую не лезь и обрезай тектс */ ; k++) {
                putChar(canvas, left + 1 + k, top + 1, text[k]);
            }
        }

        for (int y = 0; y < height; y++) {
            cout << canvas[y] << '\n';
        }
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

    Point2D a(3, 2);
    test.check(a == Point2D(3, 2) && a != Point2D(5, 2), "Point2D == and !=");
    test.check(Vector2D(4, 1) + Vector2D(1, 1) == Vector2D(5, 2), "Vector2D +");
    test.check(Vector2D(4, 1) - Vector2D(1, 1) == Vector2D(3, 0), "Vector2D -");
    test.check(a + Vector2D(4, 1) == Point2D(7, 3), "Point2D + Vector2D");
    test.check(Point2D(7, 3) - a == Vector2D(4, 1), "Point2D - Point2D");
    Point2D moved = a + Vector2D(1, 1);
    test.check(a == Point2D(3, 2) && moved != a, "Point2D is immutable");

    Rect r(Point2D(0, 0), 10, 5);
    test.check(r.contains(Point2D(0, 0)) && r.contains(Point2D(9, 4)), "Rect contains border cells");
    test.check(!r.contains(Point2D(10, 4)) && !r.contains(Point2D(-1, 2)), "Rect excludes outside points");
    test.check(r == Rect(Point2D(0, 0), 10, 5) && r != Rect(Point2D(1, 0), 10, 5), "Rect == and !=");
    bool thrown = false;
    try {
        Rect bad(Point2D(0, 0), 0, 5);
    }
    catch (const invalid_argument&) {
        thrown = true;
    }
    test.check(thrown, "Rect with zero width is rejected");

    Form form;
    form.add(make_unique<Button>(Rect(Point2D(0, 0), 16, 3)));
    form.add(make_unique<CheckBox>(Rect(Point2D(20, 0), 18, 3)));
    form.add(make_unique<TextField>(Rect(Point2D(0, 4), 22, 3)));
    form.add(make_unique<TextField>(Rect(Point2D(26, 4), 20, 3)));

    form.click(Point2D(45, 15));
    test.check(form.elementAt(Point2D(45, 15)) == -1 && form.focusedIndex() == -1, "Click on empty place changes nothing");

    form.click(Point2D(3, 1));
    test.check(form.elementText(0) == "Button:1" && form.focusedIndex() == -1, "Button counts click, takes no focus");

    form.typeText("lost");
    test.check(form.elementText(2) == "TextField", "Text without focus goes nowhere");

    form.click(Point2D(25, 1));
    test.check(form.elementText(1) == "[x] CheckBox", "CheckBox toggles on click");

    form.click(Point2D(3, 5));
    form.typeText("Alice");
    test.check(form.focusedIndex() == 2 && form.elementText(2) == "Alice", "Field gets focus and text");

    form.focusNext();
    test.check(form.focusedIndex() == 3, "focusNext goes to next field");
    form.focusNext();
    test.check(form.focusedIndex() == 2, "focusNext skips button and checkbox, wraps around");

    form.setEnabled(3, false);
    form.focusNext();
    test.check(form.focusedIndex() == 2, "focusNext skips disabled field");

    form.setEnabled(0, false);
    form.click(Point2D(3, 1));
    test.check(form.elementText(0) == "Button:1", "Disabled button ignores click");

    form.setEnabled(2, false);
    test.check(form.focusedIndex() == -1, "Disabled field loses focus");

    form.setEnabled(2, true);
    form.click(Point2D(3, 5));
    form.remove(2);
    test.check(form.focusedIndex() == -1 && form.count() == 3, "Removing focused element removes focus");

    Form overlap;
    overlap.add(make_unique<Button>(Rect(Point2D(0, 0), 10, 3)));
    overlap.add(make_unique<Button>(Rect(Point2D(5, 0), 10, 3)));
    overlap.click(Point2D(7, 1));
    test.check(overlap.elementText(1) == "Button:1" && overlap.elementText(0) == "Button:0", "Only top element gets click");

    test.printResult();

    Form demo;
    demo.add(make_unique<Button>(Rect(Point2D(0, 0), 16, 3)));
    demo.add(make_unique<CheckBox>(Rect(Point2D(20, 0), 18, 3)));
    demo.add(make_unique<TextField>(Rect(Point2D(0, 4), 22, 3)));
    demo.add(make_unique<TextField>(Rect(Point2D(26, 4), 20, 3)));
    demo.draw(50, 8);

    demo.click(Point2D(3, 1));
    demo.click(Point2D(25, 1));
    demo.click(Point2D(3, 5));
    demo.typeText("Alice");
    demo.draw(50, 8);
}