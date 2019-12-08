
// Standard
#include <atomic>
#include <execution>

#include "Meshes.h"


Vertex::Vertex()
{
	// do nothing
}

Vertex::Vertex(float x, float y, float z)
{
	this->pos.x = x;
	this->pos.y = y;
	this->pos.z = z;
}

Vertex::Vertex(glm::vec3 const& pos)
{
	this->pos = pos;
}

Vertex::Vertex(glm::vec3 const& pos, glm::vec4 const& color)
{
	this->pos = pos;
	this->color = color;
}

void Vertex::registerEdge(Edge* e)
{
	this->link_edges_count += 1;
	this->link_edges.push_front(e);
}

void Edge::registerPoly(Poly* p)
{
	this->link_polys_count += 1;
	this->link_polys.push_front(p);
}

Side::Side(Vertex* v, Edge* e)
{
	this->v = v;
	this->e = e;
}

bool Poly::getFirstWinding()
{
	for (Side& ps : poly_sides) {
		Edge* e = ps.e;

		for (Poly* other_p : e->link_polys) {

			if (other_p != this) {

				for (Side& other_ps : other_p->poly_sides) {

					// on the shared edge between the this and neighbouring
					if (ps.e == other_ps.e) {

						// Counter Clock Wise
						if (ps.v == other_ps.v) {
							return false;
						}
						// Clock Wise
						return true;
					}

					// imagine gears rotating to understand the above if
				}
			}				
		}
	}
	return true;
}

float distBetweenPos(glm::vec3 pos1, glm::vec3 pos2)
{
	float x_diff = pos1.x - pos2.x;
	float y_diff = pos1.y - pos2.y;
	float z_diff = pos1.z - pos2.z;

	return std::sqrtf(x_diff * x_diff - y_diff * y_diff - z_diff * z_diff);
}

void Poly::tesselate(LinkageMesh* me)
{
	auto ps0 = poly_sides.begin();
	auto ps1 = std::next(ps0);
	auto ps2 = std::next(ps1);

	Vertex* v0 = ps0->v;
	Vertex* v1 = ps1->v;
	Vertex* v2 = ps2->v;

	if (poly_sides_count == 3) {

		tess_tris.resize(1);

		if (getFirstWinding()) {	
			tess_tris[0].vs[0] = v0;
			tess_tris[0].vs[1] = v1;
			tess_tris[0].vs[2] = v2;
		}
		else {
			tess_tris[0].vs[0] = v2;
			tess_tris[0].vs[1] = v1;
			tess_tris[0].vs[2] = v0;
		}

		me->ttris_count += 1;
	}
	else if (poly_sides_count == 4) {

		tess_tris.resize(2);		
		Vertex* v3 = std::next(ps2)->v;
		Vertex* aux_v0;
		Vertex* aux_v1;

		// ensure v0, v1, v2, v3 are clockwise
		if (!getFirstWinding()) {

			aux_v0 = v0;
			aux_v1 = v1;
			v0 = v3;
			v1 = v2;
			v2 = aux_v1;
			v3 = aux_v0;
		}

		// shortest path tesselation
		if (distBetweenPos(v0->pos, v2->pos) <
			distBetweenPos(v1->pos, v3->pos))
		{
			tess_tris[0].vs[0] = v0;
			tess_tris[0].vs[1] = v2;
			tess_tris[0].vs[2] = v3;

			tess_tris[1].vs[0] = v0;
			tess_tris[1].vs[1] = v1;
			tess_tris[1].vs[2] = v2;
		}
		else {
			tess_tris[0].vs[0] = v0;
			tess_tris[0].vs[1] = v1;
			tess_tris[0].vs[2] = v3;

			tess_tris[1].vs[0] = v1;
			tess_tris[1].vs[1] = v2;
			tess_tris[1].vs[2] = v3;
		}

		me->ttris_count += 2;
	}
	else {
		std::cout << "Error tesselation of NPolygon not supported" << std::endl;
		return;
	}
}

//LinkageMesh::LinkageMesh()
//{
//	// 
//}
//
//LinkageMesh::LinkageMesh(const LinkageMesh&)
//{
//	//
//}
