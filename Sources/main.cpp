#include <iostream>
#include <vector>
#include "delaunay.hpp"
#include "random.hpp"
#include "Display.hpp"

int main( void )
{
	std::cout << "Hello World!" << std::endl;

	Display display;
	display.start();
	// unsigned seed = 1503;

	// std::vector<Vertex> vertices;

	// for (int i = 0; i < 100; ++i) {
	// 	vertices.push_back(Vertex(Random::randomFloat(seed), Random::randomFloat(seed)));
	// }
	// // vertices.push_back(Vertex(0, 0));
	// // vertices.push_back(Vertex(0, 1));
	// // vertices.push_back(Vertex(1, 0.5));
	// // vertices.push_back(Vertex(-1, 0.5));

	// std::vector<Triangle> delaunay = triangulate(vertices);

	// // for (auto &t : delaunay) {
	// // 	// std::cout << "sizeof triang is " << sizeof(t) << std::endl;
	// // 	std::cout << "delaunay triangle at " << t.getV0().getX() << ", " << t.getV0().getY() << " - " << t.getV1().getX() << ", " << t.getV1().getY() << " - " << t.getV2().getX() << ", " << t.getV2().getY() << std::endl;
	// // }
	return (0);
}
