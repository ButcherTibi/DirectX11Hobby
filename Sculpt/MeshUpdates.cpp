
// Header
#include "SculptMesh.hpp"

// Microsoft
#include <ppl.h>

// Mine
#include "Renderer.hpp"

// Debug
#include "RenderDocIntegration.hpp"


using namespace scme;
namespace conc = concurrency;
using namespace std::chrono_literals;


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

		r.poly_normal_updates.clear();

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

					if (poly->tesselation_type == 0) {
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
