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
uniform float EdgeThresholdTwo;
uniform int Pass;
uniform float Weight[5];

layout (binding = 0) uniform sampler2D RenderTex;
layout (binding = 1) uniform sampler2D RenderTexTwo;
layout (binding = 2) uniform sampler2D AtlasTex;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

layout (location = 0) out vec4 FragColour;

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
    vec3 texColour = texture(AtlasTex, TexCoord).rgb;

    vec3 s = normalize(vec3(lightDir[index]));
 
    float sDotN = max(dot(s, n), 0.0);

    //multiply the material properties with the texture
    vec3 diffuse = lights[index].Ld * Material.Kd * texColour * sDotN;
    vec3 ambient = lights[index].La * Material.Ka * texColour * sDotN;
    //vec3 diffuse = lights[index].Ld * Material.Kd * sDotN;
    //vec3 ambient = lights[index].La * Material.Ka * sDotN;

    vec3 spec = vec3(0.0);

    
    if(sDotN > 0) {
        vec3 v = normalize(ViewDir);
        vec3 h = normalize(v + s);
        spec = lights[index].Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }


    return ambient + diffuse + spec;
}

vec4 pass1() {

    vec3 Colour = vec3(0.0);
    for(int i = 0; i < 3; i++) {
        Colour += blinnphong(i, normalize(Normal), Position);
    }

    vec4 finalColour = vec4(Colour, 1.0f);

    return finalColour;
}


float luminance( vec3 color ) {
    return dot(lum, color);
}

vec4 pass2() {
    ivec2 pix = ivec2(gl_FragCoord.xy); //we grab a pixel to check if edge
    //pick neighboutring pixels for convolution filter
    //check lecture slides
    float s00 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,1)).rgb);
    float s10 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,0)).rgb);
    float s20 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,1)).rgb);
    float s21 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,-1)).rgb);
    float s02 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,1)).rgb);
    float s12 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,0)).rgb);
    float s22 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,-1)).rgb);
    float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
    float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
    float g = sx * sx + sy * sy;

    if( g > EdgeThreshold ) {
        return vec4(1.0, 1.0, 0.0, 1.0); //edge 1
    } else if ( g > EdgeThresholdTwo) {
        return vec4(1.0, 1.0, 1.0, 1.0); //edge 2
    } else {
        return vec4(0.0, 0.0, 0.0, 1.0); //no edge
    }
}



vec4 pass3() {
    ivec2 pix = ivec2( gl_FragCoord.xy );
    vec4 sum = texelFetch(RenderTexTwo, pix, 0) * Weight[0];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,1) ) * Weight[1];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,-1) ) * Weight[1];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,2) ) * Weight[2];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,-2) ) * Weight[2];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,3) ) * Weight[3];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,-3) ) * Weight[3];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,4) ) * Weight[4];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(0,-4) ) * Weight[4];
    
    return sum;
}

vec4 pass4() {
    ivec2 pix = ivec2( gl_FragCoord.xy );
    vec4 sum = texelFetch(RenderTex, pix, 0) * Weight[0];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(1,0) ) * Weight[1];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(-1,0) ) * Weight[1];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(2,0) ) * Weight[2];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(-2,0) ) * Weight[2];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(3,0) ) * Weight[3];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(-3,0) ) * Weight[3];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(4,0) ) * Weight[4];
    sum += texelFetchOffset( RenderTexTwo, pix, 0, ivec2(-4,0) ) * Weight[4];
   
    return sum;
}

void main() {

    if(Pass == 1) {
        FragColour = pass1();
    } else if (Pass == 2) {
        FragColour = pass2();
    } else if (Pass == 3) {
        FragColour = pass3();
    } else if (Pass == 4) {
        FragColour = pass4() + pass2();
    }
}
