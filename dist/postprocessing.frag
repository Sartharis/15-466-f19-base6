#version 330

in vec2 TexCoords;
uniform sampler2D TEX0; //IMG;
uniform sampler2D TEX1; //FRAME; // only used when TASK==2: the originally rendered frame
uniform sampler2D TEX2; //HIGHLIGHT; // used as shadow in 3
uniform vec2 TEX_OFFSET;

// 0: blur horizontally; 
// 1: blur vertically; 
// 2: combine result and draw to screen
// 3: copy to screen by combining albedo & shadow
// 4: miscellaneous (debug use)
uniform int TASK; 
out vec4 fragColor;

bool is_light(vec4 col) {
  return col.a==1 && (col.r==1 || col.g==1 || col.b==1);
}

float luminance(vec4 col) {
	return col.a * ( col.r*0.299 + col.g*0.587 + col.b*0.114 );
}

vec4 over(vec4 elem, vec4 canvas) {
  vec4 elem_ = vec4(elem.rgb * elem.a, elem.a);
  float ca = 1 - (1-elem_.a) * (1-canvas.a);
  float cr = (1-elem_.a) * canvas.r + elem_.r;
  float cg = (1-elem_.a) * canvas.g + elem_.g;
  float cb = (1-elem_.a) * canvas.b + elem_.b;
  return vec4(cr, cg, cb, ca);
}

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
  if (TASK == 0 || TASK == 1) { // TEX0: input img to be blurred
    vec2 tex_offset = 1.0 / textureSize(TEX0, 0);
    vec4 result = texture(TEX0, TexCoords) * weight[0];
    if (TASK == 0) { // horizontal blur
      for(int i = 1; i < 5; ++i) {
        result += texture(TEX0, TexCoords + vec2(tex_offset.x * i, 0)) * weight[i];
        result += texture(TEX0, TexCoords - vec2(tex_offset.x * i, 0)) * weight[i];
      }
    } else if (TASK == 1) { // vertical blur
      for(int i = 1; i < 5; ++i) {
        result += texture(TEX0, TexCoords + vec2(0, tex_offset.y * i)) * weight[i];
        result += texture(TEX0, TexCoords - vec2(0, tex_offset.y * i)) * weight[i];
      }
    }
    fragColor = result;
  } else if (TASK == 2) { // (currently not in use)
    vec4 firstpass = texture(TEX1, TexCoords);
    vec4 highlight = texture(TEX2, TexCoords);
    vec4 tex = texture(TEX0, TexCoords);
    if (is_light(highlight)) {
      fragColor = firstpass + tex * 0.5;
    } else {
      fragColor = firstpass + tex;
    }
  } else if (TASK == 3) { // toon shade + pixelate + maybe outline + combine w aura
    vec4 firstpass = texture(TEX0, TexCoords);
		float firstpass_b = luminance(firstpass);
		float up_b = luminance(texture(TEX0, TexCoords + vec2(0, -TEX_OFFSET.y)));
		float down_b = luminance(texture(TEX0, TexCoords + vec2(0, TEX_OFFSET.y)));
		float left_b = luminance(texture(TEX0, TexCoords + vec2(-TEX_OFFSET.x, 0)));
		float right_b = luminance(texture(TEX0, TexCoords + vec2(TEX_OFFSET.x, 0)));
    vec4 shadow = texture(TEX1, TexCoords);
		vec4 aura = texture(TEX2, TexCoords);
    // combine albedo & shadow
    fragColor = (shadow.a > 0 ? shadow : firstpass) + aura;

    //---- edge detection
		// is edge if current px is threshold amt darker than at least one of its neighbors
		float threshold = 0.04;
		bool is_edge = up_b - firstpass_b > threshold // up brightness larger than firstpass brightness
			|| down_b - firstpass_b > threshold
			|| left_b - firstpass_b > threshold
			|| right_b - firstpass_b > threshold;
		if (is_edge) fragColor -= vec4(0.2, 0.15, 0.1, 0);

  } else if (TASK == 4) { // debug use
    vec4 tex = texture(TEX0, TexCoords);
    fragColor = tex;
  } else {
    fragColor = vec4(1,1,1,1);
  }
}
