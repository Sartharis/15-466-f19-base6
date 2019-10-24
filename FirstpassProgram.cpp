#include "FirstpassProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include <fstream>
#include "data_path.hpp"

Scene::Drawable::Pipeline firstpass_program_pipeline;

Load< FirstpassProgram > firstpass_program(LoadTagEarly, []() -> FirstpassProgram const * {
	FirstpassProgram *ret = new FirstpassProgram();

	//----- build the pipeline template -----
	firstpass_program_pipeline.program = ret->program;

	firstpass_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	firstpass_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	firstpass_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

	//make a 1-pixel white texture to bind by default:
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	firstpass_program_pipeline.textures[0].texture = tex;
  firstpass_program_pipeline.textures[0].target = GL_TEXTURE_2D;

	return ret;
});

FirstpassProgram::FirstpassProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
  std::ifstream vertex_fs(data_path("firstpass.vert"));
  std::string vert_content( 
      (std::istreambuf_iterator<char>(vertex_fs)), std::istreambuf_iterator<char>() );

  std::ifstream fragment_fs(data_path("firstpass.frag"));
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
  CUSTOM_COL_vec4 = glGetUniformLocation(program, "CUSTOM_COL");
}

FirstpassProgram::~FirstpassProgram() {
	glDeleteProgram(program);
	program = 0;
}

