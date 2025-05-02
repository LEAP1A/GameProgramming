uniform sampler2D diffuse;
uniform vec2 spotlightLeft;
uniform vec2 spotlightRight;

varying vec2 texCoordVar;
varying vec2 varPosition;

const vec2 spotlightDir = vec2(0.0, 1.0);

const float cutoff = 0.94;

float attenuate(float dist, float a, float b)
{
    return 1.0 / (1.0 + a * dist + b * dist * dist);
}

float spotlightIntensity(vec2 lightPos, vec2 fragPos)
{
    vec2 toFrag = normalize(fragPos - lightPos);
    float angleFactor = dot(toFrag, spotlightDir);

    if (angleFactor < cutoff) return 0.0;

    float dist = distance(lightPos, fragPos);
    float brightness = attenuate(dist, 1.0, 0.5);
    return brightness * angleFactor;
}

void main()
{
    float intensityL = spotlightIntensity(spotlightLeft, varPosition);
    float intensityR = spotlightIntensity(spotlightRight, varPosition);
    float totalIntensity = intensityL + intensityR;

    vec4 color = texture2D(diffuse, texCoordVar);
    
    float boost = 1.0 + totalIntensity * 3.0;

    gl_FragColor = vec4(color.rgb * boost, color.a);
}
