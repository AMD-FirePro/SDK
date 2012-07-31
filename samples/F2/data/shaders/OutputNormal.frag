#version 420 core
in vec3 v_Normal;
out vec4 color;
void main()
{
//	vec3 normal = normalize(gl_ModelViewMatrix * v_Normal).xyz;
	vec3 normal = normalize(v_Normal);
	normal = normal*0.5 + 0.5;
	color = vec4(normal.x, normal.y, normal.z, 1.0);
}
