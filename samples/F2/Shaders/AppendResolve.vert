#version 420 core
precision highp float;
in vec2 position;
void main ( void )
{ 
	//gl_Position = vec4(mix(vec2(-1.0), vec2(3.0), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0.0, 1.0);
	gl_Position = vec4(position, 0, 1);
}