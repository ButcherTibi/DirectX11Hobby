
// Header
#include "SculptMesh.hpp"

#include "Renderer.hpp"
#include <ppl.h>

// Debug
#include "RenderDocIntegration.hpp"


using namespace scme;
namespace conc = concurrency;


void SculptMesh::markAllVerticesForNormalUpdate()
{
	modified_verts.resize(verts.size());

	uint32_t i = 0;
	for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {

		ModifiedVertex& modified_vertex = modified_verts[i];
		modified_vertex.idx = iter.index();
		modified_vertex.state = ModifiedVertexState::UPDATE;

		i++;
	}

	this->dirty_vertex_normals = true;
}

void SculptMesh::uploadVertexAddsRemoves()
{
	if (verts.size()) {

		// add vertices
		gpu_verts.resize(verts.capacity() + 1);

		for (scme::ModifiedVertex& modified_v : modified_verts) {

			// remove vertices
			if (modified_v.state == ModifiedVertexState::DELETED) {

				GPU_MeshVertex gpu_v;
				gpu_v.normal.x = 999'999.f;

				gpu_verts.upload(modified_v.idx + 1, gpu_v);
				break;
			}
		}
	}

	this->dirty_vertex_list = false;
}

void SculptMesh::uploadVertexPositions()
{
	assert_cond(dirty_vertex_list == false);

	if (modified_verts.size() > 0) {

		auto& r = renderer;

		// Counting
		{
			uint32_t group_count = 1;
			uint32_t thread_count = 0;

			for (scme::ModifiedVertex& modified_v : modified_verts) {

				if (modified_v.state == ModifiedVertexState::UPDATE &&
					verts.isDeleted(modified_v.idx) == false)
				{
					thread_count++;

					if (thread_count == 64) {
						group_count++;
						thread_count = 0;
					}
				}
			}

			r.vert_pos_updates.resize(group_count);
		}

		uint32_t group_idx = 0;
		uint32_t thread_idx = 0;

		for (scme::ModifiedVertex& modified_v : modified_verts) {

			if (modified_v.state == ModifiedVertexState::UPDATE &&
				verts.isDeleted(modified_v.idx) == false)
			{
				auto& update = r.vert_pos_updates[group_idx];
				update.vertex_id[thread_idx] = modified_v.idx + 1;
				update.new_pos[thread_idx] = dxConvert(verts[modified_v.idx].pos);

				thread_idx++;

				if (thread_idx == 64) {
					group_idx++;
					thread_idx = 0;
				}
			}
		}

		// Round Down Threads
		uint32_t last_thread = thread_idx;
		for (thread_idx = last_thread; thread_idx < 64; thread_idx++) {
			r.vert_pos_updates[group_idx].vertex_id[thread_idx] = 0;
		}

		// Load
		r.gpu_vert_pos_updates.upload(r.vert_pos_updates);

		// Command List
		auto& ctx = r.im_ctx3;
		ctx->ClearState();

		// Shader Resources
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				r.gpu_vert_pos_updates.getSRV()
			};
			ctx->CSSetShaderResources(0, srvs.size(), srvs.data());
		}

		// UAVs
		{
			std::array<ID3D11UnorderedAccessView*, 1> uavs = {
				gpu_verts.getUAV()
			};
			ctx->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), nullptr);
		}

		ctx->CSSetShader(r.update_vertex_positions_cs.Get(), nullptr, 0);

		ctx->Dispatch(r.vert_pos_updates.size(), 1, 1);
	}

	this->dirty_vertex_pos = false;
}

void SculptMesh::uploadVertexNormals()
{
	assert_cond(dirty_tess_tris == false);

	if (modified_verts.size() > 0) {

		auto& r = renderer;

		// Counting
		{
			uint32_t group_count = 1;
			uint32_t thread_count = 0;

			for (scme::ModifiedVertex& modified_v : modified_verts) {

				if (modified_v.state == ModifiedVertexState::UPDATE &&
					verts.isDeleted(modified_v.idx) == false)
				{
					thread_count++;

					if (thread_count == 64) {
						group_count++;
						thread_count = 0;
					}
				}
			}

			r.vert_normal_updates.resize(group_count);
		}

		uint32_t group_idx = 0;
		uint32_t thread_idx = 0;

		for (scme::ModifiedVertex& modified_v : modified_verts) {

			if (modified_v.state == ModifiedVertexState::UPDATE &&
				verts.isDeleted(modified_v.idx) == false)
			{
				calcVertexNormal(modified_v.idx);

				// Load
				auto& update = r.vert_normal_updates[group_idx];
				update.vertex_id[thread_idx] = modified_v.idx + 1;
				update.new_normal[thread_idx] = dxConvert(verts[modified_v.idx].normal);

				// Increment
				thread_idx++;

				if (thread_idx == 64) {
					group_idx++;
					thread_idx = 0;
				}
			}
		}

		// Round Down Threads
		uint32_t last_thread = thread_idx;
		for (thread_idx = last_thread; thread_idx < 64; thread_idx++) {
			r.vert_normal_updates[group_idx].vertex_id[thread_idx] = 0;
		}

		// Load
		r.gpu_vert_normal_updates.upload(r.vert_normal_updates);

		// Command List
		auto& ctx = r.im_ctx3;
		ctx->ClearState();

		// Shader Resources
		{
			std::array<ID3D11ShaderResourceView*, 1> srvs = {
				r.gpu_vert_normal_updates.getSRV()
			};
			ctx->CSSetShaderResources(0, srvs.size(), srvs.data());
		}

		// UAVs
		{
			std::array<ID3D11UnorderedAccessView*, 1> uavs = {
				gpu_verts.getUAV()
			};
			ctx->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), nullptr);
		}

		ctx->CSSetShader(r.update_vertex_normals_cs.Get(), nullptr, 0);

		ctx->Dispatch(r.vert_normal_updates.size(), 1, 1);
	}

	this->dirty_vertex_normals = false;
}

void SculptMesh::uploadIndexBufferChanges()
{
	assert_cond(dirty_vertex_list == false);

	if (modified_polys.size() > 0) {

		// regardless if a poly is tris or quad, always load 6 indexes
		gpu_indexes.resize(polys.capacity() * 6);

		for (scme::ModifiedPoly& modified_poly : modified_polys) {

			switch (modified_poly.state) {
			case scme::ModifiedPolyState::UPDATE: {

				if (polys.isDeleted(modified_poly.idx)) {
					continue;
				}

				scme::Poly& poly = polys[modified_poly.idx];

				if (poly.is_tris) {

					std::array<uint32_t, 3> vs_idx;
					getTrisPrimitives(&poly, vs_idx);

					for (uint32_t i = 0; i < 3; i++) {
						vs_idx[i] += 1;
					}

					gpu_indexes.upload(6 * modified_poly.idx + 0, vs_idx[0]);
					gpu_indexes.upload(6 * modified_poly.idx + 1, vs_idx[1]);
					gpu_indexes.upload(6 * modified_poly.idx + 2, vs_idx[2]);

					uint32_t zero = 0;
					gpu_indexes.upload(6 * modified_poly.idx + 3, zero);
					gpu_indexes.upload(6 * modified_poly.idx + 4, zero);
					gpu_indexes.upload(6 * modified_poly.idx + 5, zero);
				}
				else {
					std::array<uint32_t, 4> vs_idx;
					getQuadPrimitives(&poly, vs_idx);

					for (uint32_t i = 0; i < 4; i++) {
						vs_idx[i] += 1;
					}

					// Tesselation and Normals
					if (poly.tesselation_type == 0) {

						gpu_indexes.upload(6 * modified_poly.idx + 0, vs_idx[0]);
						gpu_indexes.upload(6 * modified_poly.idx + 1, vs_idx[2]);
						gpu_indexes.upload(6 * modified_poly.idx + 2, vs_idx[3]);

						gpu_indexes.upload(6 * modified_poly.idx + 3, vs_idx[0]);
						gpu_indexes.upload(6 * modified_poly.idx + 4, vs_idx[1]);
						gpu_indexes.upload(6 * modified_poly.idx + 5, vs_idx[2]);
					}
					else {
						gpu_indexes.upload(6 * modified_poly.idx + 0, vs_idx[0]);
						gpu_indexes.upload(6 * modified_poly.idx + 1, vs_idx[1]);
						gpu_indexes.upload(6 * modified_poly.idx + 2, vs_idx[3]);

						gpu_indexes.upload(6 * modified_poly.idx + 3, vs_idx[1]);
						gpu_indexes.upload(6 * modified_poly.idx + 4, vs_idx[2]);
						gpu_indexes.upload(6 * modified_poly.idx + 5, vs_idx[3]);
					}
				}
				break;
			}

			case scme::ModifiedPolyState::DELETED: {

				uint32_t zero = 0;
				gpu_indexes.upload(6 * modified_poly.idx + 0, zero);
				gpu_indexes.upload(6 * modified_poly.idx + 1, zero);
				gpu_indexes.upload(6 * modified_poly.idx + 2, zero);

				gpu_indexes.upload(6 * modified_poly.idx + 3, zero);
				gpu_indexes.upload(6 * modified_poly.idx + 4, zero);
				gpu_indexes.upload(6 * modified_poly.idx + 5, zero);
				break;
			}
			}
		}
	}

	dirty_index_buff = false;
}

void SculptMesh::uploadTesselationTriangles(TesselationModificationBasis based_on)
{
	assert_cond(dirty_vertex_pos == false);

	if (modified_verts.size() > 0) {

		auto& r = renderer;
		
		// regardless if a poly is tris or quad, always load 2 triangles
		gpu_triangles.resize(polys.capacity() * 2);

		// Aproximate the number of triangles that need to be updates
		// because traversal is expensive
		r.poly_normal_updates.clear();
		r.poly_normal_updates.reserve(modified_verts.size() * 5);

		GPU_PolyNormalUpdateGroup* update = &r.poly_normal_updates.emplace_back();
		uint32_t thread_idx = 0;

		auto increment = [&]() {

			thread_idx++;

			if (thread_idx == 32) {
				update = &r.poly_normal_updates.emplace_back();
				thread_idx = 0;
			}
		};

		auto group_updates = [&](uint32_t poly_idx) {

			if (poly_idx != 0xFFFF'FFFF) {

				Poly* poly = &polys[poly_idx];

				if (poly->is_tris) {

					std::array<uint32_t, 3> verts_idxes;
					getTrisPrimitives(poly, verts_idxes);

					for (uint32_t i = 0; i < 3; i++) {
						verts_idxes[i] += 1;
					}

					update->tess_idxs[thread_idx][0] = poly_idx * 2;
					update->tess_idxs[thread_idx][1] = 0xFFFFFFFF;
					update->poly_verts[thread_idx][0] = verts_idxes[0];
					update->poly_verts[thread_idx][1] = verts_idxes[1];
					update->poly_verts[thread_idx][2] = verts_idxes[2];
				}
				else {
					std::array<uint32_t, 4> verts_idxes;
					getQuadPrimitives(poly, verts_idxes);

					for (uint32_t i = 0; i < 4; i++) {
						verts_idxes[i] += 1;
					}

					update->tess_idxs[thread_idx][0] = poly_idx * 2;
					update->tess_idxs[thread_idx][1] = poly_idx * 2 + 1;
					update->poly_verts[thread_idx][0] = verts_idxes[0];
					update->poly_verts[thread_idx][1] = verts_idxes[1];
					update->poly_verts[thread_idx][2] = verts_idxes[2];
					update->poly_verts[thread_idx][3] = verts_idxes[3];

					update->tess_type[thread_idx] = poly->tesselation_type;

					if (poly->tesselation_type) {
						update->tess_split_vertices[thread_idx][0] = verts_idxes[0];
						update->tess_split_vertices[thread_idx][1] = verts_idxes[2];
					}
					else {
						update->tess_split_vertices[thread_idx][0] = verts_idxes[1];
						update->tess_split_vertices[thread_idx][1] = verts_idxes[3];
					}
				}

				increment();
			}
		};

		switch (based_on){
		case scme::TesselationModificationBasis::MODIFIED_POLYS: {

			for (scme::ModifiedPoly& modified_poly : modified_polys) {

				if (modified_poly.state == ModifiedPolyState::UPDATE &&
					polys.isDeleted(modified_poly.idx) == false)
				{
					group_updates(modified_poly.idx);
				}
			}
			break;
		}

		case scme::TesselationModificationBasis::MODIFIED_VERTICES: {

			for (scme::ModifiedVertex& modified_v : modified_verts) {

				if (modified_v.state == ModifiedVertexState::UPDATE &&
					verts.isDeleted(modified_v.idx) == false)
				{
					Vertex& vertex = verts[modified_v.idx];

					if (vertex.edge == 0xFFFF'FFFF) {
						return;
					}

					// loop around the changed vertex to update all polygons connected to that vertex
					uint32_t edge_idx = vertex.edge;
					Edge* edge = &edges[edge_idx];

					do {
						group_updates(edge->p0);
						group_updates(edge->p1);

						// Iter
						edge_idx = edge->nextEdgeOf(modified_v.idx);
						edge = &edges[edge_idx];
					} while (edge_idx != vertex.edge);
				}
			}
			break;
		}
		}

		uint32_t last_thread_idx = thread_idx;
		
		for (uint32_t i = last_thread_idx; i < 32; i++) {
			update->tess_idxs[i][0] = 0xFFFFFFFF;
		}

		// Load
		r.gpu_poly_normal_updates.upload(r.poly_normal_updates);

		r.gpu_r_poly_normal_updates.resizeDiscard(r.poly_normal_updates.size());
		r.poly_r_normal_updates.resize(r.poly_normal_updates.size());

		// Command List
		auto& ctx = r.im_ctx3;
		ctx->ClearState();

		// Shader Resources
		{
			std::array<ID3D11ShaderResourceView*, 2> srvs = {
				r.gpu_poly_normal_updates.getSRV(),
				gpu_verts.getSRV()
			};
			ctx->CSSetShaderResources(0, srvs.size(), srvs.data());
		}

		// UAVs
		{
			std::array<ID3D11UnorderedAccessView*, 2> uavs = {
				gpu_triangles.getUAV(),
				r.gpu_r_poly_normal_updates.getUAV()
			};
			ctx->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), nullptr);
		}

		ctx->CSSetShader(r.update_tesselation_triangles.Get(), nullptr, 0);

		ctx->Dispatch(r.poly_normal_updates.size(), 1, 1);

		// Download Results
		r.gpu_r_poly_normal_updates.download(r.poly_r_normal_updates, r.staging_buff);
		
		// Apply results to CPU
		for (uint32_t group_idx = 0; group_idx < r.poly_normal_updates.size(); group_idx++) {

			GPU_PolyNormalUpdateGroup& updates = r.poly_normal_updates[group_idx];
			GPU_Result_PolyNormalUpdateGroup& results = r.poly_r_normal_updates[group_idx];

			for (thread_idx = 0; thread_idx < 32; thread_idx++) {

				if (group_idx == r.poly_normal_updates.size() - 1 &&
					thread_idx == last_thread_idx)
				{
					break;
				}

				Poly& poly = polys[updates.tess_idxs[thread_idx][0] / 2];
				poly.normal = results.poly_normal[thread_idx];
				poly.tess_normals[0] = results.tess_normals[thread_idx][0];
				poly.tess_normals[1] = results.tess_normals[thread_idx][1];
			}
		}
	}

	dirty_tess_tris = false;
}
//
//void SculptMesh::uploadAABBs()
//{
//	auto& r = renderer;
//
//	// Check if there are out of bounds vertices
//	float max_size = 0;
//	uint32_t calls = 0;
//	{
//		for (ModifiedVertex& modified : modified_verts) {
//
//			if (modified.state == ModifiedVertexState::UPDATE &&
//				verts.isDeleted(modified.idx) == false)
//			{
//				// find if resize is necessary
//				Vertex& vertex = verts[modified.idx];
//
//				if (std::abs(vertex.pos.x) > max_size) {
//					max_size = std::abs(vertex.pos.x);
//				}
//
//				if (std::abs(vertex.pos.y) > max_size) {
//					max_size = std::abs(vertex.pos.y);
//				}
//
//				if (std::abs(vertex.pos.z) > max_size) {
//					max_size = std::abs(vertex.pos.z);
//				}
//
//				calls++;
//			}
//		}
//	}
//
//	uint32_t thread_idx = 0;
//	uint32_t group_idx = 0;
//
//	// scale the whole AABB graph and schedule all
//	if (max_size > root_aabb_size) {
//
//		root_aabb_size = max_size;
//
//		if (verts.size() % 21 == 0) {
//			r.unplaced_verts.resize(verts.size() / 21);
//		}
//		else {
//			r.unplaced_verts.resize((verts.size() / 21) + 1);
//		}
//
//		for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {
//
//			r.unplaced_verts[group_idx].vert_idxs[thread_idx] = iter.index() + 1;
//
//			thread_idx++;
//
//			if (thread_idx == 21) {
//				group_idx++;
//				thread_idx = 0;
//			}
//		}
//	}
//	else {
//		if (calls % 21 == 0) {
//			r.unplaced_verts.resize(calls / 21);
//		}
//		else {
//			r.unplaced_verts.resize((calls / 21) + 1);
//		}
//
//		for (ModifiedVertex& modified : modified_verts) {
//
//			if (modified.state == ModifiedVertexState::UPDATE &&
//				verts.isDeleted(modified.idx) == false)
//			{
//				r.unplaced_verts[group_idx].vert_idxs[thread_idx] = modified.idx + 1;
//
//				thread_idx++;
//
//				if (thread_idx == 21) {
//					group_idx++;
//					thread_idx = 0;
//				}
//			}
//		}
//	}
//
//	// round off
//	uint32_t last_thread = thread_idx;
//	for (thread_idx = last_thread; thread_idx < 21; thread_idx++) {
//		r.unplaced_verts[group_idx].vert_idxs[thread_idx] = 0;
//	}
//
//	// Loads
//	r.gpu_unplaced_verts.upload(r.unplaced_verts);
//
//	r.placed_verts.resize(r.unplaced_verts.size());
//	r.gpu_placed_verts.resizeDiscard(r.placed_verts.size());
//
//	r.mesh_aabb_graph.setUint(GPU_AABB_Graph_Fields::ROOT_SIZE, root_aabb_size);
//	r.mesh_aabb_graph.setUint(GPU_AABB_Graph_Fields::LEVELS, aabbs_levels);
//
//	// Commands //////////////////////////////////////////////////////////////////
//	auto& ctx = r.im_ctx3;
//	ctx->ClearState();
//
//	// Constant buffers
//	{
//		std::array<ID3D11Buffer*, 1> buffs = {
//			renderer.mesh_aabb_graph.get()
//		};
//		ctx->CSSetConstantBuffers(0, buffs.size(), buffs.data());
//	}
//
//	// SRV
//	{
//		std::array<ID3D11ShaderResourceView*, 3> srvs = {
//			gpu_verts.getSRV(),
//			r.gpu_unplaced_verts.getSRV()
//		};
//		ctx->CSSetShaderResources(0, srvs.size(), srvs.data());
//	}
//
//	// UAV
//	{
//		std::array<ID3D11UnorderedAccessView*, 1> uavs = {
//			r.gpu_placed_verts.getUAV()
//		};
//		ctx->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), nullptr);
//	}
//
//	ctx->CSSetShader(renderer.distribute_AABB_verts_cs.Get(), nullptr, 0);
//
//	ctx->Dispatch(r.unplaced_verts.size(), 1, 1);
//
//	// Apply results to CPU
//	r.gpu_placed_verts.download(r.placed_verts, r.staging_buff);
//
//
//
//	// if density too large then increase aabb levels
//}
