$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_tex, 0);

void main(){
    vec3 color = texture2D(s_tex,v_texcoord0).rgb;
    vec3 brightColor = vec4_splat(0.0);
    float brightness = dot(color,vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0){
        brightColor = color;
    }
    gl_FragColor = vec4(brightColor,1.0);
}