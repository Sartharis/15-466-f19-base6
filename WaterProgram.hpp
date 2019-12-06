#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

struct WaterProgram {
  WaterProgram();
  ~WaterProgram();

  GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint Normal_vec3 = -1U;
	GLuint Color_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;

	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;
	GLuint NORMAL_TO_LIGHT_mat3 = -1U;
	GLuint TIME_float = -1U;
	GLuint DEPTH_tex = -1U;
	GLuint CANVAS_SIZE_vec2 = -1U;

};

extern Load< WaterProgram > water_program;
extern Scene::Drawable::Pipeline water_program_pipeline;
