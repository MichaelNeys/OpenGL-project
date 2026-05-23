#include "scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>
#include <cstdlib>

static glm::vec3 calculateBezierPoint(float t, const std::vector<glm::vec3>& controlPoints) {
    if (controlPoints.empty()) return glm::vec3(0.0f);
    if (controlPoints.size() == 1) return controlPoints[0];

    std::vector<glm::vec3> temp = controlPoints;
    int n = temp.size() - 1;
    for (int k = 1; k <= n; k++)
        for (int i = 0; i <= n - k; i++)
            temp[i] = (1.0f - t) * temp[i] + t * temp[i + 1];
    return temp[0];
}

void Scene::buildLUT(int lutResolution) {
    arcLengthLUT.clear();
    arcLengthLUT.push_back(0.0f);
    glm::vec3 prevPoint = calculateBezierPoint(0.0f, m_controlPoints);
    float currentLen = 0.0f;
    for (int i = 1; i <= lutResolution; i++) {
        float t = (float)i / (float)lutResolution;
        glm::vec3 pt = calculateBezierPoint(t, m_controlPoints);
        currentLen += glm::length(pt - prevPoint);
        arcLengthLUT.push_back(currentLen);
        prevPoint = pt;
    }
    totalCurveLength = currentLen;
}

float Scene::getTForDistance(float targetDistance) {
    if (targetDistance <= 0.0f) return 0.0f;
    if (targetDistance >= totalCurveLength) return 1.0f;
    for (size_t i = 0; i < arcLengthLUT.size() - 1; i++) {
        if (targetDistance >= arcLengthLUT[i] && targetDistance <= arcLengthLUT[i+1]) {
            float segLen  = arcLengthLUT[i+1] - arcLengthLUT[i];
            float segFrac = (targetDistance - arcLengthLUT[i]) / segLen;
            float t0 = (float)i       / (float)(arcLengthLUT.size() - 1);
            float t1 = (float)(i + 1) / (float)(arcLengthLUT.size() - 1);
            return t0 + segFrac * (t1 - t0);
        }
    }
    return 1.0f;
}

static float cubeVertices[] = {
    -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
     0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
     0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
     0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
    -0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,

    -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

    -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f,

     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
     0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
     0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
     0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

    -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
     0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
     0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
     0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,

    -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f
};

Scene::Scene() : lightPos(0.0f, -1.0f, -12.0f) {
    unsigned int vertexCount = sizeof(cubeVertices) / sizeof(float);
    lampMesh = new Mesh(cubeVertices, vertexCount, true);

    // Terrain
    terrain = new Terrain();

    // Skybox
    std::vector<std::string> skyFaces = {
        "../models/indigo-re-skybox/indigo_ft.jpg",
        "../models/indigo-re-skybox/indigo_bk.jpg",
        "../models/indigo-re-skybox/indigo_up.jpg",
        "../models/indigo-re-skybox/indigo_dn.jpg",
        "../models/indigo-re-skybox/indigo_rt.jpg",
        "../models/indigo-re-skybox/indigo_lf.jpg"
    };
    skybox = new Skybox(skyFaces, "../shaders/skybox.vert", "../shaders/skybox.frag");

    // Bezier curve
    std::vector<glm::vec3> bezierPoints;
    int numSegments  = 500;
    bezierPointCount = numSegments + 1;

    m_controlPoints = {
        glm::vec3(15.0f,  0.0f, -10.0f),
        glm::vec3(13.0f, -1.0f, -10.0f),
        glm::vec3(10.0f,  0.8f, -10.0f),
        glm::vec3( 5.5f,  0.0f, -10.0f),
        glm::vec3( 9.0f,  0.65f,-15.0f),
        glm::vec3( 1.0f,  0.0f, -15.0f),
        glm::vec3( 0.0f,  0.2f, -20.0f),
        glm::vec3(-0.4f,  0.6f, -20.0f),
        glm::vec3(-1.5f,  2.0f, -13.5f),
        glm::vec3(-4.0f,  1.4f,  -9.0f),
        glm::vec3(-6.0f,  0.7f,  -9.2f),
        glm::vec3(-7.0f,  0.7f,  -9.5f),
        glm::vec3(-8.5f, -1.5f, -10.0f),
        glm::vec3(-7.0f,  0.7f,  -9.5f),
        glm::vec3(-12.0f,-1.6f, -10.5f), // laag gaan voor naar buiten te sturen
        glm::vec3(-20.0f,-0.9f,  -6.0f),
        glm::vec3(-18.0f, 0.0f,  -5.0f),
        glm::vec3(-11.0f, 0.1f,  -6.5f),
        glm::vec3(-13.0f, 0.0f, -15.0f),
        glm::vec3(-11.0f, 0.5f, -16.0f),
        glm::vec3(-13.0f, 0.5f, -17.5f),
        glm::vec3(-13.0f,-0.7f, -18.0f),
        glm::vec3(-11.5f,-1.5f, -20.0f),
        glm::vec3(  5.1f,-1.5f, -35.0f),
        glm::vec3(  5.8f,-1.8f, -38.0f),
        glm::vec3(-35.0f, 0.4f, -36.0f),
        glm::vec3(-30.0f, 1.0f, -14.0f),
        glm::vec3(-19.0f, 0.5f,  -9.0f),
        glm::vec3(-14.0f, -0.5f, -18.0f),
        glm::vec3( -1.0f, 0.0f, -18.0f),
        glm::vec3(  0.0f, 0.0f, -18.0f),
        glm::vec3(  1.0f, 0.0f, -18.0f),
        glm::vec3(  4.0f, 0.0f, -18.0f),
        glm::vec3(  7.0f, 0.5f, -18.0f),
        glm::vec3( 10.0f, 1.0f, -18.0f),
        glm::vec3( 20.0f, 0.0f, -22.5f),
        glm::vec3( 19.0f,-0.5f, -25.5f),
        glm::vec3( 19.0f,-0.5f, -20.0f),
        glm::vec3( 18.0f,-2.0f, -18.5f),
        glm::vec3( 17.0f,-3.0f, -14.0f),
        glm::vec3( 17.0f,-0.7f, -14.25f),
        glm::vec3( 17.0f, 0.7f, -14.5f),
        glm::vec3( 17.0f, 1.0f, -11.0f),
        glm::vec3( 17.0f, 1.0f, -10.0f),
        glm::vec3( 15.0f, 0.0f, -10.0f),
    };
    buildLUT(1000);

    for (int i = 0; i <= numSegments; i++) {
        float t = (float)i / (float)numSegments;
        bezierPoints.push_back(calculateBezierPoint(t, m_controlPoints));
    }

    glGenVertexArrays(1, &bezierVAO);
    glGenBuffers(1, &bezierVBO);
    glBindVertexArray(bezierVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bezierVBO);
    glBufferData(GL_ARRAY_BUFFER, bezierPoints.size() * sizeof(glm::vec3), bezierPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Zorg dat de willekeurige getallen elke keer anders zijn
    srand((unsigned)time(NULL));

    // Hoeveel stuifmeel blokjes wil je in totaal over de hele route?
    int amountOfPollen = 6000; 

    for (int i = 0; i < amountOfPollen; i++) {
        // Verdeel ze netjes over de lengte van de route met de LUT
        float targetDist = (totalCurveLength / amountOfPollen) * i;
        float t = getTForDistance(targetDist);
        glm::vec3 basePos = calculateBezierPoint(t, m_controlPoints);

        // Voeg een beetje "ruis" / willekeurige spreiding toe
        float radius = 0.025f; // Hoe ver mogen ze afwijken van de middellijn?
        float randX = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randY = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randZ = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;

        // Sla de uiteindelijke, verspreide positie op
        pollenPositions.push_back(basePos + glm::vec3(randX, randY, randZ));
    }

    // Village
    for (const std::string& base : {"models/Minecraft ville", "../models/Minecraft ville"}) {
        std::string path = base + "/minecraft_ville.glb";
        if (std::filesystem::exists(path)) {
            Village = new Model(path);
            std::cout << "Village geladen: " << path << std::endl;
            break;
        }
    }
    if (!Village) std::cerr << "Village model niet gevonden!" << std::endl;

    // Bee
    for (const std::string& base : {"models/Bee-1", "../models/Bee-1"}) {
        std::string path = base + "/minecraft_bee.glb";
        if (std::filesystem::exists(path)) {
            Bee = new Model(path);
            std::cout << "Bee geladen: " << path << std::endl;
            break;
        }
    }
    if (!Bee) std::cerr << "Bee model niet gevonden!" << std::endl;

    // --- KRUISJE (CROSSHAIR) SETUP ---
    // We tekenen 2 kruisende lijnen. We vullen ook dummy-data in voor Normals en TexCoords,
    // zodat jouw lightingShader niet in de war raakt omdat hij die verwacht!
    float crosshairVertices[] = {
        // Positie (X, Y, Z) | Normals (X, Y, Z) | TexCoords (U, V)
        // Lijn 1: Horizontaal
        -0.03f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.03f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        // Lijn 2: Verticaal
         0.0f, -0.04f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.04f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    // Vertel de GPU hoe de array is opgebouwd (komt exact overeen met je kubus)
    // 0 = Positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 1 = Normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 2 = TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Scene::setLightUniforms(Shader& shader) {
    // 1. GLOBAAL LICHT (Maan)
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  glm::vec3(0.08f, 0.08f, 0.15f));
    shader.setVec3("light.diffuse",  glm::vec3(0.25f, 0.25f, 0.35f));
    shader.setVec3("light.specular", glm::vec3(0.1f,  0.1f,  0.1f));

    // 2. JE VASTE LANTAARNS (Altijd aan)
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 2.0f, 0.5f, -15.0f),
        glm::vec3(-11.0f, 0.5f, -15.0f),
        glm::vec3( 11.0f, 0.5f, -15.0f),
        glm::vec3( -2.0f, 0.5f, -15.0f),
    };
    for (int i = 0; i < 4; i++) {
        std::string n = std::to_string(i);
        shader.setVec3 ("pointLights[" + n + "].position",  pointLightPositions[i]);
        shader.setVec3 ("pointLights[" + n + "].ambient",   glm::vec3(0.05f, 0.02f, 0.0f));
        shader.setVec3 ("pointLights[" + n + "].diffuse",   glm::vec3(1.0f,  0.6f,  0.2f));
        shader.setVec3 ("pointLights[" + n + "].specular",  glm::vec3(1.0f,  0.8f,  0.5f));
        shader.setFloat("pointLights[" + n + "].constant",  1.0f);
        shader.setFloat("pointLights[" + n + "].linear",    0.09f);
        shader.setFloat("pointLights[" + n + "].quadratic", 0.032f);
    }
}

void Scene::Draw(Shader& lightingShader, Shader& lampShader,
                 glm::mat4& view, glm::mat4& projection,
                 glm::vec3& cameraPos) {

    // Skybox
    skybox->Draw(view, projection);

    // Lighting setup
    lightingShader.use();
    setLightUniforms(lightingShader);
    lightingShader.setVec3("viewPos", cameraPos);
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setInt("texture_diffuse1", 0);

    // Terrein
    terrain->Draw(lightingShader);

    // Village
    glDisable(GL_CULL_FACE);
    if (Village) {
        glm::mat4 VillageModel = glm::mat4(1.0f);
        VillageModel = glm::translate(VillageModel, glm::vec3(0.0f, -3.0f, -10.0f));
        VillageModel = glm::rotate(VillageModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        VillageModel = glm::scale(VillageModel, glm::vec3(5.0f));

        for (unsigned int i = 0; i < Village->meshes.size(); i++) {
            
            // Bepaal of we de glow shader moeten gebruiken
            bool useLampShader = false;
            if (i == 7 || i == 8) {
                useLampShader = true; // Gewone lantaarns zijn altijd AAN
            }
            if (i == 5 && redstoneLampsOn) {
                useLampShader = true; // Redstone lampen alleen als de schakelaar AAN is!
            }

            if (useLampShader) {
                lampShader.use();
                lampShader.setMat4("model", VillageModel);
                lampShader.setMat4("view", view);
                lampShader.setMat4("projection", projection);
                Village->meshes[i].Draw(lampShader);
            } else {
                lightingShader.use();
                lightingShader.setMat4("model", VillageModel);
                
                // Als de redstone lamp UIT is, wordt hij net als de muren getekend (donker)
                lightingShader.setVec3("material.ambient",  glm::vec3(0.2f, 0.2f, 0.2f));
                lightingShader.setVec3("material.diffuse",  glm::vec3(0.8f, 0.8f, 0.8f));
                lightingShader.setVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
                lightingShader.setFloat("material.shininess", 10.0f);
                Village->meshes[i].Draw(lightingShader);
            }
        }
    }
    glEnable(GL_CULL_FACE);

    // Bee
    if (Bee) {
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        float deltaTime   = currentTime - lastTime;
        lastTime = currentTime;

        currentDistance += 0.8f * deltaTime;
        if (currentDistance > totalCurveLength)
            currentDistance = fmod(currentDistance, totalCurveLength);

        float t        = getTForDistance(currentDistance);
        glm::vec3 beePos  = calculateBezierPoint(t, m_controlPoints);
        float tNext    = getTForDistance(currentDistance + 0.1f);
        if (tNext < t) tNext = 1.0f;
        glm::vec3 nextPos = calculateBezierPoint(tNext, m_controlPoints);
        glm::vec3 dir     = glm::normalize(nextPos - beePos);

        float yaw   = atan2(dir.x, dir.z) + glm::radians(180.0f);
        float pitch = atan2(dir.y, sqrt(dir.x*dir.x + dir.z*dir.z));

        glm::mat4 BeeModel = glm::mat4(1.0f);
        BeeModel = glm::translate(BeeModel, beePos);
        BeeModel = glm::rotate(BeeModel, yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
        BeeModel = glm::rotate(BeeModel, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        BeeModel = glm::scale(BeeModel, glm::vec3(0.01f));

        lightingShader.use();
        lightingShader.setMat4("model", BeeModel);
        lightingShader.setVec3("material.ambient",  glm::vec3(0.10f, 0.10f, 0.10f));
        lightingShader.setVec3("material.diffuse",  glm::vec3(0.35f, 0.35f, 0.38f));
        lightingShader.setVec3("material.specular", glm::vec3(0.45f, 0.45f, 0.48f));
        lightingShader.setFloat("material.shininess", 40.0f);
        Bee->Draw(lightingShader);
    }

    // --- Stuifmeel spoor tekenen (Pollen trail) ---
    lightingShader.use(); 
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection);
    
    // Zet de textuur uit, en kies een mooie goud/gele pollen kleur!
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setVec3("fallbackColor", glm::vec3(1.0f, 0.8f, 0.2f)); 

    // Zet culling uit voor de zekerheid, zodat we altijd alle kanten van de blokjes zien
    glDisable(GL_CULL_FACE);

    // Loop door alle gegenereerde puntjes heen
    for (const glm::vec3& pos : pollenPositions) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        
        // Geef elk blokje een willekeurige kanteling gebaseerd op zijn positie!
        // Dit breekt het "rechte" patroon en maakt het een echte wolk van deeltjes.
        model = glm::rotate(model, pos.x * 15.0f, glm::vec3(1.0f, 0.5f, 0.2f)); 
        model = glm::rotate(model, pos.z * 10.0f, glm::vec3(0.2f, 1.0f, 0.5f));
        
        // Maak ze klein
        model = glm::scale(model, glm::vec3(0.005f)); 
        
        lightingShader.setMat4("model", model);
        
        // Teken de kubus met belichting
        lampMesh->Draw(); 
    }

    // --- KRUISJE (CROSSHAIR) TEKENEN ---
    
    // 1. Zet de Depth Test uit: het kruisje wordt nu overal overheen getekend!
    glDisable(GL_DEPTH_TEST);
    
    lightingShader.use(); 
    
    // 2. MAGIE: Door de matrices op "Identity" (1.0f) te zetten, wordt het 
    // perspectief genegeerd en tekenen we direct plat op het 2D-scherm.
    lightingShader.setMat4("model", glm::mat4(1.0f));
    lightingShader.setMat4("view", glm::mat4(1.0f));
    lightingShader.setMat4("projection", glm::mat4(1.0f));
    
    // 3. Maak het kruisje mooi spierwit
    lightingShader.setBool("hasDiffuseTexture", false);
    lightingShader.setVec3("fallbackColor", glm::vec3(1.0f, 1.0f, 1.0f)); 

    // 4. Teken de 2 lijnen (4 punten)
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
    
    // 5. BELANGRIJK: Zet de Depth Test weer aan, anders tekent het volgende frame fout!
    glEnable(GL_DEPTH_TEST);
    
    // Zet culling weer netjes aan voor de rest van je programma
    glEnable(GL_CULL_FACE);
}

void Scene::Delete() {
    skybox->Delete();
    delete skybox;

    terrain->Delete();
    delete terrain;

    // Verwijder de crosshair VAO en VBO
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);

    glDeleteVertexArrays(1, &bezierVAO);
    glDeleteBuffers(1, &bezierVBO);

    lampMesh->Delete();
    delete lampMesh;

    if (Village) { Village->Delete(); delete Village; }
    if (Bee)     { Bee->Delete();     delete Bee;     }
}

// Helper functie: Test of een straal (laser) een specifieke 3D-driehoek doorboort
bool rayIntersectsTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);
    
    if (a > -EPSILON && a < EPSILON) return false; // Straal loopt parallel aan de driehoek

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * glm::dot(edge2, q);
    if (t > EPSILON) return true; // Raak! We hebben de driehoek geraakt.

    return false;
}

void Scene::checkMouseClick(glm::mat4 view, glm::mat4 projection, glm::vec3 cameraPos) {
    if (Village == nullptr || Village->meshes.size() <= 5) return;

    // 1. Omdat we een FPS camera hebben, schieten we ALTIJD door het exacte midden (0.0f, 0.0f)!
    glm::vec4 ray_clip = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f); 
    glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

    // 2. We hebben de juiste schaal/positie van het dorp nodig
    glm::mat4 VillageModel = glm::mat4(1.0f);
    VillageModel = glm::translate(VillageModel, glm::vec3(0.0f, -3.0f, -10.0f));
    VillageModel = glm::rotate(VillageModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    VillageModel = glm::scale(VillageModel, glm::vec3(5.0f));

    // 3. Haal de punten en de driehoeken van de Redstone Lampen op (Index 5)
    const std::vector<Mesh::Vertex>& verts = Village->meshes[5].getVertices();
    const std::vector<unsigned int>& indices = Village->meshes[5].getIndices();

    // 4. Test onze laserstraal tegen ELKE driehoek van de lampen
    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = verts[indices[i]].Position;
        glm::vec3 v1 = verts[indices[i+1]].Position;
        glm::vec3 v2 = verts[indices[i+2]].Position;

        v0 = glm::vec3(VillageModel * glm::vec4(v0, 1.0f));
        v1 = glm::vec3(VillageModel * glm::vec4(v1, 1.0f));
        v2 = glm::vec3(VillageModel * glm::vec4(v2, 1.0f));

        if (rayIntersectsTriangle(cameraPos, ray_wor, v0, v1, v2)) {
            // RAAK!
            redstoneLampsOn = !redstoneLampsOn;
            std::cout << "Redstone Lamp geraakt met kruisje! Status: " << (redstoneLampsOn ? "AAN" : "UIT") << std::endl;
            break; 
        }
    }
}

