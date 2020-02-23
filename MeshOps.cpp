
#include <execution>

#include "Meshes.h"


Vertex* initBlankVertex(LinkageMesh& me)
{
	me.verts_size += 1;
	Vertex& v = me.verts.emplace_front();
	v.link_edges_count = 0;

	v.self_it = me.verts.before_begin();

	return &me.verts.front();
}

Vertex* initVertex(LinkageMesh& me, glm::vec3 const& pos)
{
	Vertex* v = initBlankVertex(me);
	v->pos = pos;
	return v;
}

Vertex* initVertex(LinkageMesh& me, float const& x, float const& y, float const& z)
{
	return initVertex(me, {x, y, z});
}

Vertex* initVertex(LinkageMesh& me, glm::vec3 const& pos, glm::vec4 const& color)
{
	Vertex* v = initVertex(me, pos);
	v->color = color;

	return v;
}

Edge* initEdge(LinkageMesh& me, Vertex* v0, Vertex* v1)
{
	me.edges_size += 1;

	Edge& e = me.edges.emplace_front();
	e.verts[0] = v0;
	e.verts[1] = v1;
	e.link_polys_count = 0;

	e.self_it = me.edges.before_begin();

	// register Edge with verts
	v0->registerEdge(&e);
	v1->registerEdge(&e);
	return &e;
}

Edge* ensureEdge(LinkageMesh& me, Vertex* v0, Vertex* v1)
{
	Edge* e = findEdge(v0, v1);
	return e != nullptr ? e : initEdge(me, v0, v1);
}

Edge* findEdge(Vertex* v0, Vertex* v1)
{
	for (Edge* link_e0 : v0->link_edges) {
		for (Edge* link_e1 : v1->link_edges) {

			if (link_e0 == link_e1) {
				return link_e0;
			}
		}
	}
	return nullptr;
}

Poly* initTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2,
	Edge* e0, Edge* e1, Edge* e2)
{
	me.polys_size += 1;

	Poly& p = me.polys.emplace_front();
	p.poly_sides_count = 3;
	p.poly_sides.emplace_front(v0, e0);
	p.poly_sides.emplace_front(v1, e1);
	p.poly_sides.emplace_front(v2, e2);
	p.build(&me);
	
	p.self_it = me.polys.before_begin();

	return &me.polys.front();
}

Poly* fabricateTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2)
{
	Edge* e0 = ensureEdge(me, v0, v1);
	Edge* e1 = ensureEdge(me, v1, v2);
	Edge* e2 = ensureEdge(me, v2, v0);

	return initTris(me, v0, v1, v2, e0, e1, e2);
}

/*
	e->v0 e0 v
	+--------+
	|       /
	|      /
	|    / e1
	|  /  
	|/
	+
	e->v1
*/
//Poly* addTris(LinkageMesh& me, Edge* e, Vertex* v)
//{
//	Edge* e0 = initEdge(me, e->verts[0], v);
//	Edge* e1 = initEdge(me, e->verts[1], v);
//
//	Poly p;
//	p.poly_sides_count = 3;
//	p.poly_sides.emplace_front(e->verts[0], e0);
//	p.poly_sides.emplace_front(v, e1);
//	p.poly_sides.emplace_front(e->verts[1], e);
//
//	me.polys_size += 1;
//	me.polys.push_front(p);
//	me.polys.front().it = me.polys.before_begin();
//
//	return &me.polys.front();
//}

Poly* initQuad(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3,
	Edge* e0, Edge* e1, Edge* e2, Edge* e3)
{
	me.polys_size += 1;

	Poly& p = me.polys.emplace_front();
	p.poly_sides_count = 4;
	p.poly_sides.emplace_front(v0, e0);
	p.poly_sides.emplace_front(v1, e1);
	p.poly_sides.emplace_front(v2, e2);
	p.poly_sides.emplace_front(v3, e3);
	p.build(&me);

	p.self_it = me.polys.before_begin();

	// register Poly with Edges
	e0->registerPoly(&p);
	e1->registerPoly(&p);
	e2->registerPoly(&p);
	e3->registerPoly(&p);

	return &me.polys.front();
}

Poly* fabricateQuad(LinkageMesh& me, glm::vec3 const& vp0, glm::vec3 const& vp1,
	glm::vec3 const& vp2, glm::vec3 const& vp3,
	glm::vec3 normal, glm::vec4 const& color)
{
	Vertex* v0 = initVertex(me, vp0, color);
	Vertex* v1 = initVertex(me, vp1, color);
	Vertex* v2 = initVertex(me, vp2, color);
	Vertex* v3 = initVertex(me, vp3, color);

	v0->normal = normal;
	v1->normal = normal;
	v2->normal = normal;
	v3->normal = normal;

	Edge* e0 = initEdge(me, v0, v1);
	Edge* e1 = initEdge(me, v1, v2);
	Edge* e2 = initEdge(me, v2, v3);
	Edge* e3 = initEdge(me, v3, v0);

	return initQuad(me, v0, v1, v2, v3, 
		e0, e1, e2, e3);
}

void addTriangleListToMesh(LinkageMesh& me, std::vector<uint32_t>& indexes, VertexAtributes& attrs, bool flip_winding)
{
	std::vector<Vertex*> verts{ attrs.positions.size() };

	// Create Vertices
	for (uint64_t i = 0; i < verts.size(); i++) {
		verts[i] = initBlankVertex(me);
	}

	// Position
	for (uint64_t i = 0; i < verts.size(); i++) {
		Vertex* v = verts[i];
		v->pos = attrs.positions[i];
	}

	// UVs
	if (attrs.uvs.size()) {
		for (uint64_t i = 0; i < verts.size(); i++) {
			Vertex* v = verts[i];
			v->uv = attrs.uvs[i];
		}
	}

	// Normals
	if (attrs.normals.size()) {
		for (uint64_t i = 0; i < verts.size(); i++) {
			Vertex* v = verts[i];
			v->normal = attrs.normals[i];
		}
	}

	for (uint64_t i = 0; i + 2 < indexes.size(); i += 3) {

		uint32_t& idx0 = indexes[i];
		uint32_t& idx1 = indexes[i + 1];
		uint32_t& idx2 = indexes[i + 2];

		if (flip_winding) {
			fabricateTris(me, verts[idx2], verts[idx1], verts[idx0]);
		}
		else {
			fabricateTris(me, verts[idx0], verts[idx1], verts[idx2]);
		}	
	}
}

