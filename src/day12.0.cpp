#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace {

class Vector2D {
    int x;
    int y;

  public:
    explicit Vector2D(int x, int y);
    int getX() const;
    int getY() const;
};

Vector2D::Vector2D(int x, int y)
: x(x)
, y(y)
{
}

int Vector2D::getX() const
{
    return x;
}

int Vector2D::getY() const
{
    return y;
}

class Position: public Vector2D {
    using Vector2D::Vector2D;
};

class Heading : public Vector2D {
    using Vector2D::Vector2D;
};

Position advance(Position pos, const Heading head, int steps)
{
    pos = Position { pos.getX() + steps * head.getX(),
                     pos.getY() + steps * head.getY() };
    return pos;
}

class Pose {
    Heading heading;
    Position position;

  public:
    Pose (Position pos, Heading heading);

    Position getPosition() const;
    Heading getHeading() const;
};

Pose::Pose(const Position position, const Heading heading)
: position {position}
, heading {heading}
{
}

Position Pose::getPosition() const
{
    return position;
}

Heading Pose::getHeading() const
{
    return heading;
}

Heading clockwise(Heading heading, const int steps)
{
    for (int i = 0; i < steps; ++i) {
        heading = Heading { heading.getY(), -heading.getX() };
    }
    return heading;
}

Heading counterClockwise(Heading heading, const int steps)
{
    for (int i = 0; i < steps; ++i) {
        heading = Heading { -heading.getY(), heading.getX() };
    }
    return heading;
}

Pose clockwise(Pose pose, const int degress) {
    const auto steps = degress/90;
    pose = { pose.getPosition(), clockwise(pose.getHeading(), steps) };
    return pose;
}

Pose counterClockwise(Pose pose, const int degress)
{
    const auto steps = degress/90;
    pose = { pose.getPosition(), counterClockwise(pose.getHeading(), steps) };
    return pose;
}

Pose advance(Pose pose, const int steps)
{
    pose = { advance(pose.getPosition(), pose.getHeading(), steps),
             pose.getHeading() };
    return pose;
}

Pose move(Pose pose, const Vector2D delta)
{
    const auto& posePos = pose.getPosition();
    pose = { Position { posePos.getX() + delta.getX(),
                        posePos.getY() + delta.getY() },
             pose.getHeading() };
    return pose;
}

class Solution {

    int solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        Pose pose { Position { 0, 0 }, Heading { 1 , 0 } };
        const auto ret = loop([&](char command, uint16_t arg) {
            switch (command) {
              case 'N':
                pose = move(pose, Vector2D { 0, arg });
                break;
              case 'S':
                pose = move(pose, Vector2D { 0, -arg });
                break;
              case 'E':
                pose = move(pose, Vector2D {arg, 0 });
                break;
              case 'W':
                pose = move(pose, Vector2D {-arg, 0 });
                break;
              case 'L':
                pose = counterClockwise(pose, arg);
                break;
              case 'R':
                assert(arg % 90 == 0);
                pose = clockwise(pose, arg);
                break;
              case 'F':
                pose = advance(pose, arg);
                break;
              default:
                assert(false);
            }
            return true;
        });
        solution = std::abs(pose.getPosition().getX()) +
                   std::abs(pose.getPosition().getY());
        return ret;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    auto output() const
    {
        std::cout << "calls: " << calls << '\n';
        return solution;
    }
};

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    char command;
    uint16_t arg;
    std::string line;

    const auto ret = callable([&](auto&& body) {
        bool res;
        while (inputFile >> command >> arg && (res = body(command, arg)))
            ;
        return res;
    });
}

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    readBatches(filename, solution);
    std::cout << solution.output() << '\n';
    return 0;
}
