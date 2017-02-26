#define blend(a,b,x) ((a) * (1 - (x)) + (b) * (x))
#define whichtile(x,freq) (floor((x) * (freq)))
#define repeat(x,freq)  (mod((x) * (freq), 1.0))



#include "arman/noises.h"

surface soapBubble(float  Kd = .2;
    float Ka = .1;
    float Ks = .3;
    float Kr = .45, rim_width = .3;
    float iridescence = 1.;
    float roughness = .001;
    color specularcolor = .7;
    float transparencyGain = .35;

    float reflmapGain = 1.;
    float reflmapGamma = 1.;
    float rainbowGain = 1.;
    float rainbowGamma = 1.;
    float colorNoiseFrequency = 2.8;
    string reflectionenv = "";
      
 )
{
    normal Nf, n;
    n = normalize(N);
    Nf = faceforward(n, I);
    vector i = normalize(-I);
    point PP = transform ("shader", P);
    PP* = colorNoiseFrequency*vfBm(PP/.3 * PI, 4.5, 2, 0.05, 0.5);
    color rainbowColor = ((color(noise(2 * PI * PP) - .5)) + .5);

    // reflect
    vector Rcurrent = reflect(I, Nf);
    vector Rworld = vtransform("world", Rcurrent);
    color Cr = color environment(reflectionenv, Rworld);
    Cr = color(reflmapGain * pow(Cr[0], 1 / reflmapGamma), reflmapGain * pow(Cr[1],
       1 / reflmapGamma), reflmapGain * pow(Cr[2], 1 / reflmapGamma));

    float dot = 1 - i.Nf;

    float iridescence1 = iridescence * pow(blend(rim_width * 1.1, .9, 
      smoothstep(0,1, dot)), 1/rainbowGamma);
    Oi = transparencyGain*blend(rim_width, 1, smoothstep(0,1, dot));

    Ci = rainbowColor * rainbowGain * iridescence1 * (Ka * ambient() + Kd * diffuse(Nf) 
       + specularcolor * Ks * specular (Nf, i, roughness)) +  Kr * Cr;
    Ci* = Oi;
}
