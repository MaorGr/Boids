#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <Eigen/Geometry>


namespace bg = boost::geometry;

using namespace Eigen;
using Point = bg::model::point<float, 2, bg::cs::cartesian>;
using Box = bg::model::box<Point>;
