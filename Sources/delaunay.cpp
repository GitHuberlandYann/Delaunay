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

Vertex::Vertex( float x, float y ) : x(x), y(y) {}
bool Vertex::equals( Vertex &other ) {
	// return (nearly_equal(_x, other._x) && nearly_equal(_y, other._y));
	return (x == other.x && y == other.y);
}
float Vertex::distance( Vertex other ) {
	#if __linux__
	return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
	#else
	return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
	#endif
}
void Vertex::operator+=( Vertex other ) {
	x += other.x;
	y += other.y;
}
void Vertex::operator-=( Vertex other ) {
	x -= other.x;
	y -= other.y;
}
void Vertex::operator*=( float value ) {
	x *= value;
	y *= value;
}
void Vertex::operator/=( float value ) {
	x /= value;
	y /= value;
}
Vertex Vertex::operator+( Vertex other ) {
	return {x + other.x, y + other.y};
}
Vertex Vertex::operator-( Vertex other ) {
	return {x - other.x, y - other.y};
}
Vertex Vertex::operator*( float value ) {
	return {x * value, y * value};
}
Vertex Vertex::operator/( float value ) {
	return {x / value, y / value};
}
std::ostream &operator<<( std::ostream &out, Vertex &v )
{
	out << "Vertex " << v.x << ", " << v.y << std::endl;
	return (out);
}

class Edge {
	private:

	public:
		Vertex v0, v1;

		Edge( Vertex v0, Vertex v1 ) : v0(v0), v1(v1) {}
		bool equals( Edge &other ) {
			return ((v0.equals(other.v0) && v1.equals(other.v1)) || (v0.equals(other.v1) && v1.equals(other.v0)));
		}
};

static t_circle calcCircumCirc( Vertex v0, Vertex v1, Vertex v2 )
{
	float A = v1.x - v0.x;
	float B = v1.y - v0.y;
	float C = v2.x - v0.x;
	float D = v2.y - v0.y;

	float E = A * (v0.x + v1.x) + B * (v0.y + v1.y);
	float F = C * (v0.x + v2.x) + D * (v0.y + v2.y);

	float G = 2.0 * (A * (v2.y - v1.y) - B * (v2.x - v1.x));

	float dx, dy;

	t_circle res;

	// Collinear points, get extremes and use midpoint as center
	if (round(std::abs(G)) == 0) {
		float minx = std::min(v0.x, std::min(v1.x, v2.x));
		float miny = std::min(v0.y, std::min(v1.y, v2.y));
		float maxx = std::max(v0.x, std::max(v1.x, v2.x));
		float maxy = std::max(v0.y, std::max(v1.y, v2.y));

		res.x = (minx + maxx) * 0.5f;
		res.y = (miny + maxy) * 0.5f;

		dx = res.x - minx;
		dy = res.y - miny;
	} else {
		float cx = (D * E - B * F) / G;
		float cy = (A * F - C * E) / G;

		res.x = cx;
		res.y = cy;

		dx = res.x - v0.x;
		dy = res.y - v0.y;
	}
	res.r = (dx * dx + dy * dy);  // sqrt

	return (res);
}

Triangle::Triangle( Vertex v0, Vertex v1, Vertex v2 ) : v0(v0), v1(v1), v2(v2) {
	_circumCirc = calcCircumCirc(v0, v1, v2);
}
bool Triangle::inCircumCircle( Vertex &v ) {
	float dx = _circumCirc.x - v.x;
	float dy = _circumCirc.y - v.y;
	return ((dx * dx + dy * dy) <= _circumCirc.r);  // sqrt
}
bool Triangle::shareEdge( Triangle &t ) {
	return (v0.equals(t.v0) || v0.equals(t.v1) || v0.equals(t.v2)
		|| v1.equals(t.v0) || v1.equals(t.v1) || v1.equals(t.v2)
		|| v2.equals(t.v0) || v2.equals(t.v1) || v2.equals(t.v2));
}
float Triangle::getRadius( void ) { return (_circumCirc.r); }

std::ostream &operator<<( std::ostream &out, Triangle &t )
{
	out << "Triangle " << t.v0.x << ", " << t.v0.y << " | " << t.v1.x << ", " << t.v1.y << " | " << t.v2.x << ", " << t.v2.y << std::endl;
	return (out);
}
std::ostream &operator<<( std::ostream &out, Edge &e )
{
	out << "Edge " << e.v0.x << ", " << e.v0.y << " | " << e.v1.x << ", " << e.v1.y << std::endl;
	return (out);
}

Triangle superTriangle( std::vector<Vertex> &vertices )
{
	float minX = FLT_MAX, maxX = -minX;
	float minY = minX, maxY = maxX;

	for (auto &v : vertices) {
		minX = std::min(minX, v.x);
		minY = std::min(minY, v.y);
		maxX = std::max(maxX, v.x);
		maxY = std::max(maxY, v.y);
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
			edges.push_back({it->v0, it->v1});
			edges.push_back({it->v1, it->v2});
			edges.push_back({it->v2, it->v0});
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
			triangles.push_back({it->v0, it->v1, vertex});
			// uniqueEdges.push_back(*it); // TODO just call triangles.push_back here
		// 	std::cout << "\tunique " << *it;
		}// else std::cout << "\tdouble " << *it;
		++it;
	}

	// for (auto &e : uniqueEdges) {
	// 	// Triangle t(e.v0, e.v1, vertex);
	// 		// std::cout << "\tadd one " << t;
	// 	triangles.push_back({e.v0, e.v1, vertex});
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
	// std::cout << "super triang is " << st.v0.x << ", " << st.v0.y << " - " << st.v1.x << ", " << st.v1.y << " - " << st.v2.x << ", " << st.v2.y << std::endl;
	return (res);
}
