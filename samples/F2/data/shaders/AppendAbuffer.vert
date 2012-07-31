//-------------------------------------------------------------------------------------- 
// Name: VertexShader.glsl 
// Desc: Source of Vertex Shaders of the tessellation program
// 
// Copyright (c) 2008 Advanced Micro Devices, Inc. All rights reserved.
//--------------------------------------------------------------------------------------

#version 130
precision highp float;
out vec4 color;
void main ( void )
{ 
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    //pos = gl_Position / gl_Position.w;
    color = gl_Color;
}

