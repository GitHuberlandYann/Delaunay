#ifndef DELAUNAY_HPP
# define DELAUNAY_HPP

class Vertex {
	private:

	public:
		float x, y;

		Vertex( float x, float y );

		bool equals( Vertex &other );
		float distance( Vertex other );
		void operator+=( Vertex other );
		void operator-=( Vertex other );
		void operator*=( float value );
		void operator/=( float value );
		Vertex operator+( Vertex other );
		Vertex operator-( Vertex other );
		Vertex operator*( float value );
		Vertex operator/( float value );
};

typedef struct s_circle {
	float x = 0, y = 0, r = 0;
}				t_circle;

class Triangle {
	private:
		t_circle _circumCirc;
	
	public:
		Vertex v0, v1, v2;
		Triangle( Vertex v0, Vertex v1, Vertex v2 );

		bool inCircumCircle( Vertex &v );
		bool shareEdge( Triangle &t );
		float getRadius( void );
};

# include <fstream>

std::ostream &operator<<( std::ostream &out, Triangle &t );
std::ostream &operator<<( std::ostream &out, Vertex &v );

std::vector<Triangle> triangulate( std::vector<Vertex> &vertices );

#endif
