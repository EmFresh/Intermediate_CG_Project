#version 420

// enums
const int DEFAULT = 0;
const int POINT = 1;
const int DIRECTIONAL = 2;
const int SPOTLIGHT = 3;

uniform vec4 LightPosition;

// colour
uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;

// scalar
uniform float LightSpecularExponent;
uniform float Attenuation_Constant;
uniform float Attenuation_Linear;
uniform float Attenuation_Quadratic;

// other
uniform bool LightEnable = true;
uniform bool AmbiantEnable = true;
uniform bool DiffuseEnable = true;
uniform bool SpecularEnable = true;
uniform int LightType;
uniform vec3 LightDirection;
uniform float LightAngleConstraint;

// uniform sampler2D uTex;
uniform sampler2D uPosOP;
uniform sampler2D uPosTrans;
uniform sampler2D uNormOP;
uniform sampler2D uNormTrans;
uniform sampler2D uScene;
uniform sampler2D uRamp;
uniform vec4 uViewPos;

uniform bool toonActiveDiff;
uniform bool toonActiveSpec;

in vec2 texcoord;

out vec4 outColor;

void pointLight();
void directionalLight();
vec3 calculatePointLight();

void defaultLight() { directionalLight(); }

void spotLight() 
{
  

}

vec3 blinnPhong(vec3 lightDir, vec3 viewDir, vec3 pos, vec3 norm )
{
  //vec3 pos   = (texture(uPosOP,  texcoord)).rgb;//frag position  
  //vec3 norm    = (texture(uNormOP, texcoord)).rgb;
  vec3 halfDir = normalize(lightDir + viewDir);
  vec3 diffuse, specular;

  //Blinn-Phong Lighting here:
  
  //Diffuse   
  diffuse = max(dot(norm, lightDir),0.0)  * LightDiffuse * int(DiffuseEnable);
  diffuse *= ( toonActiveDiff ? texture(uRamp,vec2(max(dot(norm, lightDir),0.0))).rgb : vec3(1)) ;

  //Specular  
  specular = pow(max(dot(norm, halfDir),0.0), LightSpecularExponent) *
    LightSpecular * int(SpecularEnable);
  specular *=  (toonActiveSpec ? texture(uRamp,vec2(pow(max(dot(norm, halfDir),0.0), LightSpecularExponent))).rgb : vec3(1)) ;
 
  return (diffuse + specular) * int(LightEnable);
}

vec3 calculatePointLight(){

  //variables  
  vec3 posOP     = (texture(uPosOP    , texcoord)).rgb;//frag position
  vec3 posTrans  = (texture(uPosTrans , texcoord)).rgb;//frag position
  vec3 normOP    = (texture(uNormOP   , texcoord)).rgb;
  vec3 normTrans = (texture(uNormTrans, texcoord)).rgb;
  vec3 lightDirOP  = normalize(LightPosition.xyz - posOP);
  vec3 lightDirTrans = normalize(LightPosition.xyz - posTrans);
  vec3 viewDirOP   = uViewPos.xyz - posOP;   
  vec3 viewDirTrans   = uViewPos.xyz - posTrans;   
  
  //Atenuation calculation
  float distOP= length(LightPosition.xyz - posOP);
  float distTrans= length(LightPosition.xyz - posTrans);
  float attenuationOP = ( 1.0 / (Attenuation_Constant + Attenuation_Linear * distOP + Attenuation_Quadratic * (distOP * distOP)));
  float attenuationTrans = ( 1.0 / (Attenuation_Constant + Attenuation_Linear * distTrans + Attenuation_Quadratic * (distTrans * distTrans)));
  
  //return 
  return
  blinnPhong(lightDirOP, viewDirOP, posOP, normOP) * attenuationOP +  
  blinnPhong(lightDirTrans, viewDirTrans, posTrans, normTrans) * attenuationTrans;

}

void pointLight() {   
 outColor.rgb += calculatePointLight();
 //outColor =(texture(uScene, texcoord));
 }

vec3 calculateDirectionalLight() {
  vec3 norm = (texture(uNormOP, texcoord)).rgb;
  vec3 pos  = (texture(uPosOP, texcoord)).rgb;
  
  
 // float diff = max(dot(norm, lightDir), 0.0);

 // vec3 reflectDir = reflect(-lightDir, norm);
  vec3 viewDir    = uViewPos.xyz - pos;
  //float spec = pow(max(dot(viewDir, reflectDir), 0), LightSpecularExponent);

  vec3 diffuse = LightDiffuse;
  vec3 specular = LightSpecular;

  //Blinn-Phong Lighting here:
  
  //Diffuse 
  vec3 lightDir = normalize(-LightDirection);
 diffuse = max(dot(norm, lightDir),0.0) * LightDiffuse;

  //Specular
  vec3 halfDir    = normalize(lightDir + viewDir);
  specular = pow(max(dot(viewDir, halfDir),0.0), LightSpecularExponent) *  LightSpecular;


  
  return diffuse + specular;
}

void directionalLight() {
 
  outColor.rgb += calculateDirectionalLight();
}


void main() {

  vec3 colour = texture(uScene, texcoord).rgb;

  // Ambient Light
  float ambientStrength = 1.0;
  outColor.rgb = colour * LightAmbient * ambientStrength * int(AmbiantEnable);

 
    switch (LightType) {
    case POINT:
      pointLight();
      break;
    case DIRECTIONAL:
      directionalLight();
      break;
    case SPOTLIGHT:
      spotLight();
      break;
    default:
      defaultLight();
    }


  //outColor = normalize(texture(uPosOP,texcoord));
  //outColor.rgb = abs(texture(uNormOP,texcoord).rgb);
  outColor.a = 1;
}