#include <math.h>
#include <float.h>
#include <vector>
#include "delaunay.hpp"
#include <iostream>
#include "Benchmark.hpp"
#include "utils.hpp"

// static bool nearly_equal(
//   float a, float b,
//   float epsilon = 128 * FLT_EPSILON, float abs_th = FLT_MIN)
//   // those defaults are arbitrary and could be removed
// {
//   assert(std::numeric_limits<float>::epsilon() <= epsilon);
//   assert(epsilon < 1.f);

//   if (a == b) return true;

//   auto diff = std::abs(a-b);
//   auto norm = std::min((std::abs(a) + std::abs(b)), std::numeric_limits<float>::max());
//   // or even faster: std::min(std::abs(a + b), std::numeric_limits<float>::max());
//   // keeping this commented out until I update figures below
//   return diff < std::max(abs_th, epsilon * norm);
// }

Vertex::Vertex( float x, float y ) : _x(x), _y(y) {}
bool Vertex::equals( Vertex &other ) {
	// return (nearly_equal(_x, other._x) && nearly_equal(_y, other._y));
	return (_x == other._x && _y == other._y);
}
float Vertex::getX( void ) { return (_x); }
float Vertex::getY( void ) { return (_y); }

class Edge {
	private:
		Vertex _v0, _v1;

	public:
		Edge( Vertex v0, Vertex v1 ) : _v0(v0), _v1(v1) {}
		bool equals( Edge &other ) {
			return ((_v0.equals(other._v0) && _v1.equals(other._v1)) || (_v0.equals(other._v1) && _v1.equals(other._v0)));
		}
		Vertex getV0( void ) { return (_v0); }
		Vertex getV1( void ) { return (_v1); }
};

static t_circle calcCircumCirc( Vertex v0, Vertex v1, Vertex v2 )
{
	float A = v1.getX() - v0.getX();
	float B = v1.getY() - v0.getY();
	float C = v2.getX() - v0.getX();
	float D = v2.getY() - v0.getY();

	float E = A * (v0.getX() + v1.getX()) + B * (v0.getY() + v1.getY());
	float F = C * (v0.getX() + v2.getX()) + D * (v0.getY() + v2.getY());

	float G = 2.0 * (A * (v2.getY() - v1.getY()) - B * (v2.getX() - v1.getX()));

	float dx, dy;

	t_circle res;

	// Collinear points, get extremes and use midpoint as center
	if (round(std::abs(G)) == 0) {
		float minx = std::min(v0.getX(), std::min(v1.getX(), v2.getX()));
		float miny = std::min(v0.getY(), std::min(v1.getY(), v2.getY()));
		float maxx = std::max(v0.getX(), std::max(v1.getX(), v2.getX()));
		float maxy = std::max(v0.getY(), std::max(v1.getY(), v2.getY()));

		res.x = (minx + maxx) * 0.5f;
		res.y = (miny + maxy) * 0.5f;

		dx = res.x - minx;
		dy = res.y - miny;
	} else {
		float cx = (D * E - B * F) / G;
		float cy = (A * F - C * E) / G;

		res.x = cx;
		res.y = cy;

		dx = res.x - v0.getX();
		dy = res.y - v0.getY();
	}
	res.r = (dx * dx + dy * dy);  // sqrt

	return (res);
}

Triangle::Triangle( Vertex v0, Vertex v1, Vertex v2 ) : _v0(v0), _v1(v1), _v2(v2) {
	_circumCirc = calcCircumCirc(v0, v1, v2);
}
bool Triangle::inCircumCircle( Vertex &v ) {
	float dx = _circumCirc.x - v.getX();
	float dy = _circumCirc.y - v.getY();
	return ((dx * dx + dy * dy) <= _circumCirc.r);  // sqrt
}
bool Triangle::shareEdge( Triangle &t ) {
	return (_v0.equals(t._v0) || _v0.equals(t._v1) || _v0.equals(t._v2)
		|| _v1.equals(t._v0) || _v1.equals(t._v1) || _v1.equals(t._v2)
		|| _v2.equals(t._v0) || _v2.equals(t._v1) || _v2.equals(t._v2));
}
Vertex Triangle::getV0( void ) { return (_v0); }
Vertex Triangle::getV1( void ) { return (_v1); }
Vertex Triangle::getV2( void ) { return (_v2); }
float Triangle::getRadius( void ) { return (_circumCirc.r); }

std::ostream &operator<<( std::ostream &out, Triangle &t )
{
	out << "Triangle " << t.getV0().getX() << ", " << t.getV0().getY() << " | " << t.getV1().getX() << ", " << t.getV1().getY() << " | " << t.getV2().getX() << ", " << t.getV2().getY() << std::endl;
	return (out);
}
std::ostream &operator<<( std::ostream &out, Edge &e )
{
	out << "Edge " << e.getV0().getX() << ", " << e.getV0().getY() << " | " << e.getV1().getX() << ", " << e.getV1().getY() << std::endl;
	return (out);
}

Triangle superTriangle( std::vector<Vertex> &vertices )
{
	float minX = FLT_MAX, maxX = -minX;
	float minY = minX, maxY = maxX;

	for (auto &v : vertices) {
		minX = std::min(minX, v.getX());
		minY = std::min(minY, v.getY());
		maxX = std::max(maxX, v.getX());
		maxY = std::max(maxY, v.getY());
	}

	float dx = (maxX - minX) * 10;
	float dy = (maxY - minY) * 10;

	Vertex v0(minX - dx, minY - dy * 3);
	Vertex v1(minX - dx, maxY + dy);
	Vertex v2(maxX + dx * 3, maxY + dy);
	return Triangle(v0, v1, v2);
}

static void addVertex( std::vector<Triangle> &triangles, Vertex vertex )
{
	// std::cout << "DEBUG addVertex" << std::endl;
	std::vector<Edge> edges;

	// rm triangles with circumCircle containing the vertex
	for (auto it = triangles.begin(); it != triangles.end();) {
		if (it->inCircumCircle(vertex)) {
			edges.push_back({it->getV0(), it->getV1()});
			edges.push_back({it->getV1(), it->getV2()});
			edges.push_back({it->getV2(), it->getV0()});
			// std::cout << "\trm one " << *it;
			it = triangles.erase(it);
		} else {
			// std::cout << "\tkeep one " << *it;
			++it;
		}
	}
	
	// rm double edges
	for (auto it = edges.begin(); it != edges.end();) {
		bool unique = true;
		for (auto itbis = edges.begin(); itbis != edges.end(); ++itbis) {
			if (it != itbis && it->equals(*itbis)) {
				unique = false;
			// std::cout << "\trm one edge" << std::endl;
				break ;
			}
		}
		if (unique) {
			triangles.push_back({it->getV0(), it->getV1(), vertex});
			// uniqueEdges.push_back(*it); // TODO just call triangles.push_back here
		// 	std::cout << "\tunique " << *it;
		}// else std::cout << "\tdouble " << *it;
		++it;
	}

	// for (auto &e : uniqueEdges) {
	// 	// Triangle t(e.getV0(), e.getV1(), vertex);
	// 		// std::cout << "\tadd one " << t;
	// 	triangles.push_back({e.getV0(), e.getV1(), vertex});
	// }
}

// https://www.gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation/
std::vector<Triangle> triangulate( std::vector<Vertex> &vertices )
{
	std::vector<Triangle> res;

    // Bench b;
	Triangle st = superTriangle(vertices);
	res.push_back(st);
    // b.stamp("super triangle");

	for (auto &v : vertices) {
		addVertex(res, v);
	}
    // b.stamp("addVertex");
	// std::cout << "DEBUG cleanup" << std::endl;
	// rm triangles that share edge with superTriangle
	for (auto it = res.begin(); it != res.end();) {
		if (it->shareEdge(st)) {
			it = res.erase(it);
			// std::cout << "rm one triang" << std::endl;
		} else {
			// std::cout << "keep one " << *it << std::endl;
			++it;
		}
	}
    // b.stamp("rm shared edges");
	// res.push_back(st);
	// std::cout << "super triang is " << st.getV0().getX() << ", " << st.getV0().getY() << " - " << st.getV1().getX() << ", " << st.getV1().getY() << " - " << st.getV2().getX() << ", " << st.getV2().getY() << std::endl;
	return (res);
}
