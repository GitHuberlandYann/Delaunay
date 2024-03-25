#ifndef DELAUNAY_HPP
# define DELAUNAY_HPP

class Vertex {
	private:
		float _x, _y;

	public:
		Vertex( float x, float y );

		bool equals( Vertex &other );
		float getX( void );
		float getY( void );
};

typedef struct s_circle {
	float x = 0, y = 0, r = 0;
}				t_circle;

class Triangle {
	private:
		Vertex _v0, _v1, _v2;
		t_circle _circumCirc;
	
	public:
		Triangle( Vertex v0, Vertex v1, Vertex v2 );

		bool inCircumCircle( Vertex &v );
		bool shareEdge( Triangle &t );
		Vertex getV0( void );
		Vertex getV1( void );
		Vertex getV2( void );
};

std::vector<Triangle> triangulate( std::vector<Vertex> &vertices );

#endif
