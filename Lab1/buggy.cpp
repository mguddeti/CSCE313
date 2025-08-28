#include <cstring> // required for memcpy
#include <iostream>

struct Point {
    int x, y;

    Point() : x(), y() {}
    Point(int _x, int _y) : x(_x), y(_y) {}
};

class Shape {
public: // in C++, class members are private by default, explicitly add the public identifier
    int vertices;
    Point **points;

    Shape(int _vertices) {
        vertices = _vertices;
        points = new Point *[vertices + 1]; // array of Point pointers
    }

    ~Shape() {
        delete[] points; // destructor for heap-allocated variables
    }

    void addPoints(Point pts[]) {

        for (int i = 0; i <= vertices; i++) {
            // the source address is bad
            points[i] = new Point();
            memcpy(points[i], &pts[i % vertices], sizeof(Point));
        }
    }

    double area() {
        int twiceArea = 0;
        for (int i = 0; i < vertices; ++i) {
            int j = (i + 1) % vertices;
            twiceArea += points[i]->x * points[j]->y - points[j]->x * points[i]->y;
        }
        return abs(twiceArea / 2.0);
    }
};

int main() {
    // FIXME: create the following points using the three different methods
    //        of defining structs:
    //          tri1 = (0, 0)
    //          tri2 = (1, 2)
    //          tri3 = (2, 0)

    Point tri1 = {0, 0};
    Point tri2 = {1, 2};
    Point tri3 = {2, 0};

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape *tri = new Shape(3);
    tri->addPoints(triPts);

    // FIXME: create the following points using your preferred struct
    //        definition:
    //          quad1 = (0, 0)
    //          quad2 = (0, 2)
    //          quad3 = (2, 2)
    //          quad4 = (2, 0)

    // adding points to quad
    Point quad1 = {0, 0};
    Point quad2 = {0, 2};
    Point quad3 = {2, 2};
    Point quad4 = {2, 0};
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape *quad = new Shape(4);
    quad->addPoints(quadPts);

    // FIXME: print out area of tri and area of quad
    std::cout << tri->area() << std::endl;
    std::cout << quad->area() << std::endl;
}
