#version 460

const float PI = 3.14159265359;

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

layout (location = 3) in vec3 ParticlePosition;
layout (location = 4) in vec3 ParticleVelocity;
layout (location = 5) in float ParticleAge;
layout (location = 6) in vec2 ParticleRotation;

out vec3 PPosition;
out vec3 Velocity;
out float Age;
out vec2 Rotation;

uniform int ParticlePass;

out vec2 TexCoord;
out vec3 fNormal;
out vec3 fPosition;

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

uniform float Time;
uniform float DeltaT;
uniform vec3 Accel;
uniform float ParticleLifetime;
uniform vec3 Emitter = vec3(0);
uniform mat3 EmitterBasis;
uniform float ParticleSize;

uniform sampler1D RandomTex;

uniform mat4 Proj;
uniform mat3 NormalMatrix;
uniform mat4 MV;
uniform mat4 MVP; 

vec3 randomInitialVelocity() {
    float theta = mix(0.0, PI / 6.0, texelFetch(RandomTex, 4 * gl_VertexID, 0).r);
    float phi = mix(0.0, 2.0 * PI, texelFetch(RandomTex, 4 * gl_VertexID + 1, 0).r);
    float velocity = mix(1.25, 1.5, texelFetch(RandomTex, 4 * gl_VertexID + 2, 0).r);


    vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));

    return normalize(EmitterBasis * v) * velocity;
}

float randomInitialRotationalVelocity() {
    return mix(-15.0, 15.0, texelFetch(RandomTex, 4 * gl_VertexID + 3, 0).r);
}

void update() {
    if(ParticleAge < 0 || ParticleAge > ParticleLifetime) {
        PPosition = Emitter;
        Velocity = randomInitialVelocity();
        Rotation = vec2(0.0, randomInitialRotationalVelocity());

        if(ParticleAge < 0) {
            Age = ParticleAge + DeltaT;
        } else {
            Age = (ParticleAge - ParticleLifetime) + DeltaT;
        }

    } else {
        PPosition = ParticlePosition + ParticleVelocity * DeltaT;
        Velocity = ParticleVelocity + Accel * DeltaT;
        Rotation.x = mod(ParticleRotation.x + ParticleRotation.y * DeltaT, 2.0 * PI);
        Rotation.y = ParticleRotation.y;
        Age = ParticleAge + DeltaT;
    }
}

void render() {
    float cs = cos(ParticleRotation.x);
    float sn = sin(ParticleRotation.x);

    mat4 RotationAndTranslation = mat4(
        1, 0, 0, 0, 
        0, cs, sn, 0, 
        0, -sn, cs, 0, 
        ParticlePosition.x, ParticlePosition.y, ParticlePosition.z, 1
    );

    mat4 m = MV * RotationAndTranslation;
    fPosition = (m * vec4(VertexPosition, 1)).xyz;
    fNormal = (m * vec4(VertexNormal, 0)).xyz;
    
    
    gl_Position = Proj * vec4(fPosition, 1.0);


}

void otherRender() {

    TexCoord = VertexTexCoord;
    vec3 pos = vec3(MV * vec4(VertexPosition, 1.0));
    
    fPosition = pos; 

    // 3 lights, 3 directions
    for(int i =  0; i < 3; i++) {
        //lightDir[i].direction = toObjectLocal * (lights[i].Position.xyz - pos);
        lightDir[i].direction = lights[i].Position.xyz - pos;
    }

    ViewDir = normalize(pos);
    
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    //gl_Position = Proj * vec4(fPosition, 1.0);


}

void main()
{
    if(ParticlePass == 1) {
        update();
    } else if(ParticlePass == 2){
        render();
    } else {
        otherRender();
    }
}
