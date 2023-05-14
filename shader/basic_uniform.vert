#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
//layout (location = 2) in vec2 VertexTexCoord;

layout (location = 3) in vec4 VertexTangent;

//out vec2 TexCoord;
out vec3 Normal;
out vec4 Position;

out vec3 ViewDir;

uniform struct LightInfo {
    vec4 Position;
    vec3 La;    //ambient
    vec3 Ld;    //diffuse
    vec3 Ls;    //specular
} lights[3];

//output 3 lightdirections for normal calculations
out struct LightDirection {
    vec3 direction;
} lightDir[3];

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP; 

void main()
{
    //TexCoord = VertexTexCoord;
    Normal = normalize(NormalMatrix * VertexNormal);

    //calculations for normal map according to lab sheet
    //vec3 norm = normalize(NormalMatrix * VertexNormal);
    //vec3 tang = normalize(NormalMatrix * vec3(VertexTangent));
    //vec3 binormal = normalize(cross(norm, tang)) * VertexTangent.w;
    
    //mat3 toObjectLocal = mat3(tang.x, binormal.x, norm.x, tang.y, binormal.y, norm.y, tang.z, binormal.z, norm.z );


    vec3 pos = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0));

    // 3 lights, 3 directions
    for(int i =  0; i < 3; i++) {
        //lightDir[i].direction = toObjectLocal * (lights[i].Position.xyz - pos);
        lightDir[i].direction = lights[i].Position.xyz - pos;
    }

    ViewDir = normalize(pos);

    gl_Position = MVP * vec4(VertexPosition, 1.0);
}
