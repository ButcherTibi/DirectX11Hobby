#include "./SculptMesh.hpp"

// Microsoft
#include <ppl.h>

// GLM
#include <glm/gtx/compatibility.hpp>

// Mine
#include <App/Application.hpp>

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
		_stroke_value = local_value;
	}
	else {
		_stroke_value = global_value * factor;
	}
}

template<>
void BrushProperty<uint32_t>::calcStrokeValue(uint32_t& global_value)
{
	if (local) {
		_stroke_value = local_value;
	}
	else {
		_stroke_value = global_value;
	}
}

template<>
void BrushProperty<BrushFalloff>::calcStrokeValue(BrushFalloff& global_value)
{
	if (local) {
		_stroke_value = local_value;
	}
	else {
		_stroke_value.type = global_value.type;
		_stroke_value.spread = global_value.spread * factor.spread;
		_stroke_value.steepness = global_value.steepness * factor.steepness;
	}
}

template<>
void BrushProperty<float>::calcSpeedInfluence(float& value, float speed)
{
	if (speed_influence.enable) {
		value *= speed_influence.calcFactor(speed);
	}
}


void SculptMesh::applyStandardBrush(
	glm::vec3&,
	BrushProperty<uint32_t>&,
	BrushProperty<float>& stroke_radius,
	BrushProperty<float>& stroke_strength,
	BrushProperty<BrushFalloff>&,
	std::vector<BrushStep>& raw_steps, uint32_t start_step)
{
	//printf(__func__);
	//printf("\n");

	float steps_strength = stroke_strength._stroke_value / (raw_steps.size() - start_step);

	for (uint32_t i = start_step; i < raw_steps.size(); i++) {

		BrushStep& step = raw_steps[i];

		//printf("  target = %.2f %.2f %.2f \n", step.target.x, step.target.y, step.target.z);

		// calculate speed change from step to step
		//float speed;
		//float inv_speed;

		//if (i == 0) {
		//	speed = 0;
		//	inv_speed = 1;
		//}
		//else {
		//	float delta_distance = glm::distance(steps[i - 1].mouse_pos, steps[i].mouse_pos);
		//	
		//	if (delta_distance < 5) {
		//		//continue;
		//	}
		//}

		float radius = stroke_radius._stroke_value;

		// Calculate strength
		float strength = steps_strength;
		strength *= app.delta_time;

		// traverse vertices
		sphereIsectAABBs(step.target, radius);

		for (VertexBoundingBox* aabb : _traced_aabbs) {
			for (uint32_t vertex_idx : aabb->verts) {
				if (vertex_idx != 0xFFFF'FFFF) {

					Vertex& vertex = verts[vertex_idx];
					float distance = glm::distance(vertex.pos, step.target);

					if (distance < radius) {

						vertex.pos.x += strength;
						markVertexMoved(vertex_idx);
					}
				}
			}
		}

		uploadVertexPositions();
	}
}
