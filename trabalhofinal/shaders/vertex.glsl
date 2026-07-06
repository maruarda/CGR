#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragUV;

void main() {
    /* posição do fragmento em coordenadas de mundo (para iluminação) */
    fragPos = vec3(model * vec4(aPos, 1.0));

    /* normal transformada pela matriz normal (inversa-transposta do model)
     * — aqui simplificamos assumindo que 'model' não tem escala não-uniforme;
     * se adicionar escala distorcida no futuro, trocar por transpose(inverse(model)) */
    fragNormal = mat3(model) * aNormal;

    fragUV = aUV;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
