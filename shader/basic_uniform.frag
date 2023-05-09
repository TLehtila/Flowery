#version 460

in vec2 TexCoord;

in vec3 ViewDir;
in vec3 Normal;
in vec4 Position;

//take in the 3 light directions
in struct LightDirection {
    vec3 direction;
} lightDir[3];

uniform float EdgeThreshold;
uniform int Pass;

layout (binding = 0) uniform sampler2D flowerTex;
layout (binding = 1) uniform sampler2D leafTex;

layout (location = 0) out vec4 FragColor;

uniform struct LightInfo {
    vec4 Position;
    vec3 La;    //ambient
    vec3 Ld;    //diffuse
    vec3 Ls;    //specular
} lights[3];

uniform struct MaterialInfo {
    vec3 Ka;    //ambient
    vec3 Kd;    //diffuse
    vec3 Ks;    //specular
    float Shininess;
} Material;

vec3 blinnphong(int index, vec3 n, vec4 pos) {
    vec3 texColour = texture(flowerTex, TexCoord).rgb;

    vec3 s = normalize(vec3(lightDir[index]));
 
    float sDotN = max(dot(s, n), 0.0);

    //multiply the material properties with the texture
    vec3 diffuse = lights[index].Ld * Material.Kd * texColour * sDotN;
    vec3 ambient = lights[index].La * Material.Ka * texColour * sDotN;

    vec3 spec = vec3(0.0);

    
    if(sDotN > 0) {
        vec3 v = normalize(ViewDir);
        vec3 h = normalize(v + s);
        spec = lights[index].Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }


    return ambient + diffuse + spec;
}

void main() {
    vec3 Colour = vec3(0.0);

    for(int i = 0; i < 3; i++) {
        Colour += blinnphong(i, normalize(Normal), Position);
    }

    FragColor = vec4(Colour, 1.0);

}
