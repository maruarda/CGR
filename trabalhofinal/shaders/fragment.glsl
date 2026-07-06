#version 330 core
in vec3 fragPos;
in vec3 fragNormal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
    vec3 N = normalize(fragNormal);

    /* --- ambient --- */
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;

    /* --- diffuse (Lambert) --- */
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    /* --- specular (Blinn-Phong) --- */
    vec3 V = normalize(viewPos - fragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
