#version 330 core


#ifdef GL_ES
precision mediump float;
#endif

#define S(a, b, t) smoothstep(a, b, t)

uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;

//in vec2 uv;

float TaperBox(vec2 p, float wb, float wt, float yb, float yt, float blur){
    float m = S(-blur, blur,p.y - yb);
    p.x = abs(p.x);
    m *= S(blur, -blur,p.y - yt);
    float w = mix(wb, wt, (p.y - yb) / (yt - yb));
    m *= S(blur, -blur, p.x - w);
    return m;
}

vec4 Tree(vec2 uv,vec3 color, float blur){
    float m = TaperBox(uv, .03, .03, -.05, .25, blur);
    m += TaperBox(uv, .2, .1, .25, .5, blur);
    m += TaperBox(uv, .15, .05, .5, .75, blur);
    m += TaperBox(uv, .1, .0, .75, 1.0, blur);

    float shadow = TaperBox(uv-vec2(0.2, 0.0), 0.1, 0.5, 0.20, 0.26, blur);
    shadow += TaperBox(uv-vec2(-0.25, 0.0), 0.1, 0.5, 0.45, 0.5, blur);
    shadow += TaperBox(uv-vec2(0.25, 0.0), 0.1, 0.5, 0.7, 0.75, blur);
    // m -= shadow*0.8;
    color -= shadow*.8;
    return vec4(color , m);
}

float GetHeight(float x){
    return (sin(x*.43)+sin(x)*.6 + .4*sin(x*0.2)+.1*sin(.09*x))/2.0;
}

vec4 Layer(vec2 uv, float blur){
    vec4 col = vec4(0.0,0.0,0.0,0.0);
    float id = floor(uv.x);
    float y = GetHeight(uv.x);
    float ground = S(blur, -blur, uv.y+y); //ground
    col += ground;
    uv.x = fract(uv.x) - 0.5;
    float n = fract(sin(id*234.12)*53213.4)*2.0 - 1.0;
    float x = n*.3;
    y = -GetHeight(id+x+.5);
    vec4 tree = Tree((uv-vec2(x,y))*vec2(1.0, 1.0+0.3*n),vec3(1.0), blur);
    // col.rg = uv;

    col = mix(col, tree, tree.a);
    col.a = max(tree.a, ground);
    return col;
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float twinkle(vec2 uv, float noise){
    return noise*sin(u_time*cos(uv.x*uv.y*noise)+noise*100.0);
}

void main(){
    
    
    vec2 uv = (gl_FragCoord.xy - .5*u_resolution)/u_resolution.y;
    float blur = 0.005;
    vec2 M = (2.0*u_mouse/u_resolution.xy) - 1.0;
    // uv.y += M.y*.01;
    float t = 0.3*u_time;
    // uv *= 8.0;


    vec4 col = vec4(0.0,0.0,0.0,1.0);
    float noise = pow(rand(uv),100.0);
    vec2 m_pos = vec2(0.5,0.2);
    vec2 m_diff = vec2(0.07);
    float moon = S(0.01,-0.01,length(uv - m_pos)-0.15);
    moon *= S(-0.007,0.030,length(uv - m_diff - m_pos)-0.15);
    col += twinkle(uv, noise)*(1.0-moon);
    col += moon;
    // col += Layer(uv, blur);
    vec4 layer;
    for(float i=0.0;i<1.0; i+= 1.0/10.0){
        float scale = mix(30.0, 1.0, i);
        blur = mix(0.05, 0.005, i);
        layer = Layer(uv*scale + vec2(t+sin(i*30.0),0.5)-M/5.0,blur);
        layer.rgb *= (1.0-i);
        col = mix(col, layer, layer.a);
    }
    layer = Layer(uv + vec2(t,1.0)-M/5.0,0.04);
    layer.rgb *= 0.0;
    col = mix(col, layer, layer.a);
    col += 0.3*S(1.0,0.0,length(uv - m_pos));
    gl_FragColor = vec4(col.rgb, 1.0);
}
