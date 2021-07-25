
// Header
#include "SculptMesh.hpp"

// Microsoft
#include <ppl.h>

// GLM
#include <glm/gtx/compatibility.hpp>

// Mine
#include "Renderer.hpp"

// Debug
#include "RenderDocIntegration.hpp"


using namespace scme;
namespace conc = concurrency;
using namespace std::chrono_literals;


float calcAlpha(float min, float max, float val)
{
	if (val < min) {
		return 0;
	}

	if (val > max) {
		return 1;
	}

	return (val - min) / (max - min);
}

float BrushSpeedInfluence::calcFactor(float speed)
{
	float alpha = calcAlpha(min_speed, max_speed, speed);
	return glm::lerp(min_factor, max_factor, alpha);
}

template<>
void BrushProperty<float>::calcStrokeValue(float& global_value)
{
	if (local) {
		_value = local_value;
	}
	else {
		_value = global_value * factor;
	}
}

template<>
void BrushProperty<BrushFalloff>::calcStrokeValue(BrushFalloff& global_value)
{
	if (local) {
		_value = local_value;
	}
	else {
		_value.type = global_value.type;
		_value.spread = global_value.spread * factor.spread;
		_value.steepness = global_value.steepness * factor.steepness;
	}
}

template<>
void BrushProperty<float>::calcStepValue(float speed)
{
	// @HERE
	if (speed_influence.enable) {
		_value = speed_influence.calcFactor(speed);
	}
}


void SculptMesh::applyStandardBrush(
	glm::vec3& ray_origin,
	BrushProperty<float>& radius,
	BrushProperty<float>& strength,
	BrushProperty<BrushFalloff>& falloff,
	std::vector<BrushStep>& steps, uint32_t start_step)
{
	printf(__func__);
	printf("\n");

	for (uint32_t i = start_step; i < steps.size(); i++) {

		BrushStep& step = steps[i];

		//printf("  target = %.2f %.2f %.2f \n", step.target.x, step.target.y, step.target.z);

		// calculate speed change from step to step
		float speed;

		if (i == 0) {
			speed = 0;
		}
		else {
			float delta_distance = glm::distance(steps[i - 1].target, steps[i].target);
			uint32_t delta_duration = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(
				steps[i].time - steps[i - 1].time).count();

			speed = delta_distance / delta_duration;
		}

		radius.calcStepValue(speed);
		strength.calcStepValue(speed);

		// traverse vertices
		sphereIsectAABBs(step.target, radius._value);

		for (VertexBoundingBox* aabb : _traced_aabbs) {
			for (uint32_t vertex_idx : aabb->verts) {
				if (vertex_idx != 0xFFFF'FFFF) {

					Vertex& vertex = verts[vertex_idx];
					float distance = glm::distance(vertex.pos, step.target);

					if (distance < radius._value) {

						vertex.pos.x += 0.25f;
						markVertexMoved(vertex_idx);
					}
				}
			}
		}

		/*for (auto iter = verts.begin(); iter != verts.end(); iter.next()) {

			Vertex& vertex = iter.get();
			vertex.pos.x += 0.5f;

			markVertexMoved(iter.index());
		}*/

		uploadVertexPositions();
	}
}
