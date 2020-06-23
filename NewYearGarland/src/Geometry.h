#pragma once

template<typename T>
struct Point
{
    T x;
    T y;
};

using IPoint = Point<int>;

template<typename T>
struct Dimensions
{
    T width;
    T height;
};

using IDimensions = Dimensions<int>;