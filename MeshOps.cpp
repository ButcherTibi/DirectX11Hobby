
#include <execution>

#include "Meshes.h"


Vertex* createLoneVertex(LinkageMesh& me, float const& x, float const& y, float const& z)
{
	me.verts_size += 1;
	me.verts.push_front(Vertex(x, y, z));
	me.verts.front().it = me.verts.before_begin();

	return &me.verts.front();
}

Vertex* createLoneVertex(LinkageMesh& me, glm::vec3 const& pos)
{
	me.verts_size += 1;
	me.verts.push_front(Vertex(pos));
	me.verts.front().it = me.verts.before_begin();

	return &me.verts.front();
}

Vertex* createLoneVertex(LinkageMesh& me, glm::vec3 const& pos, glm::vec4 const& color)
{
	me.verts_size += 1;
	me.verts.push_front(Vertex(pos, color));
	me.verts.front().it = me.verts.before_begin();

	return &me.verts.front();
}

Edge* bridgeVerts(LinkageMesh& me, Vertex* v0, Vertex* v1)
{
	Edge e;
	e.verts[0] = v0;
	e.verts[1] = v1;
	e.link_polys_count = 0;
	
	me.edges_size += 1;
	me.edges.push_front(e);
	me.edges.front().it = me.edges.before_begin();

	return &me.edges.front();
}

Poly* createTris(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2,
	Edge* e0, Edge* e1, Edge* e2)
{
	Poly p;
	p.poly_sides_count = 3;
	p.poly_sides.emplace_front(v0, e0);
	p.poly_sides.emplace_front(v1, e1);
	p.poly_sides.emplace_front(v2, e2);
	p.tesselate(&me);

	me.polys_size += 1;
	me.polys.push_front(p);
	me.polys.front().it = me.polys.before_begin();

	return &me.polys.front();
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
Poly* createTris(LinkageMesh& me, Edge* e, Vertex* v)
{
	Edge* e0 = bridgeVerts(me, e->verts[0], v);
	Edge* e1 = bridgeVerts(me, e->verts[1], v);

	Poly p;
	p.poly_sides_count = 3;
	p.poly_sides.emplace_front(e->verts[0], e0);
	p.poly_sides.emplace_front(v, e1);
	p.poly_sides.emplace_front(e->verts[1], e);
	p.tesselate(&me);

	me.polys_size += 1;
	me.polys.push_front(p);
	me.polys.front().it = me.polys.before_begin();

	return &me.polys.front();
}

Poly* createQuad(LinkageMesh& me, Vertex* v0, Vertex* v1, Vertex* v2, Vertex* v3,
	Edge* e0, Edge* e1, Edge* e2, Edge* e3)
{
	Poly p;
	p.poly_sides_count = 4;
	p.poly_sides.emplace_front(v0, e0);
	p.poly_sides.emplace_front(v1, e1);
	p.poly_sides.emplace_front(v2, e2);
	p.poly_sides.emplace_front(v3, e3);
	p.tesselate(&me);

	me.polys_size += 1;
	me.polys.push_front(p);
	me.polys.front().it = me.polys.before_begin();

	return &me.polys.front();
}

Poly* createLoneQuad(LinkageMesh& me, glm::vec3 const& vp0, glm::vec3 const& vp1,
	glm::vec3 const& vp2, glm::vec3 const& vp3, glm::vec4 const& color)
{
	Vertex* v0 = createLoneVertex(me, vp0, color);
	Vertex* v1 = createLoneVertex(me, vp1, color);
	Vertex* v2 = createLoneVertex(me, vp2, color);
	Vertex* v3 = createLoneVertex(me, vp3, color);

	Edge* e0 = bridgeVerts(me, v0, v1);
	Edge* e1 = bridgeVerts(me, v1, v2);
	Edge* e2 = bridgeVerts(me, v2, v3);
	Edge* e3 = bridgeVerts(me, v3, v0);

	return createQuad(me, v0, v1, v2, v3, 
		e0, e1, e2, e3);
}
