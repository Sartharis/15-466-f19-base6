#include "WaterProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include <fstream>
#include <iostream>
#include "data_path.hpp"

Scene::Drawable::Pipeline water_program_pipeline;

Load< WaterProgram > water_program(LoadTagEarly, []() -> WaterProgram const * {
	WaterProgram *ret = new WaterProgram();
	
	//----- build the pipeline template -----
	water_program_pipeline.program = ret->program;
	water_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	water_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	water_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;
  return ret;
});

WaterProgram::WaterProgram() {
  //Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
  std::ifstream vertex_fs(data_path("water.vert"));
  std::string vert_content( 
      (std::istreambuf_iterator<char>(vertex_fs)), std::istreambuf_iterator<char>() );

  std::ifstream fragment_fs(data_path("water.frag"));
  std::string frag_content( 
      (std::istreambuf_iterator<char>(fragment_fs)), std::istreambuf_iterator<char>() );

  program = gl_compile_program(
    //vertex shader:
    vert_content,
    //fragment shader:
    frag_content
  );

  //look up the locations of vertex attributes:
  Position_vec4 = glGetAttribLocation(program, "Position");
  Normal_vec3 = glGetAttribLocation(program, "Normal");
  Color_vec4 = glGetAttribLocation(program, "Color");
  TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

  //look up the locations of uniforms:
  OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
  OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
  NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");
  TIME_float = glGetUniformLocation(program, "TIME");
}

WaterProgram::~WaterProgram() {
  glDeleteProgram(program);
  program = 0;
}

