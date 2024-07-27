#include "Common.h"
#include "OpenGLCommon.h"
#include "OpenGLMarkerObjects.h"
#include "OpenGLBgEffect.h"
#include "OpenGLMesh.h"
#include "OpenGLViewer.h"
#include "OpenGLWindow.h"
#include "TinyObjLoader.h"
#include "OpenGLSkybox.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>
#include <string>

#ifndef __Main_cpp__
#define __Main_cpp__

#ifdef __APPLE__
#define CLOCKS_PER_SEC 100000
#endif

class MyDriver : public OpenGLViewer
{
    std::vector<OpenGLTriangleMesh *> mesh_object_array;
    OpenGLBgEffect *bgEffect = nullptr;
    OpenGLSkybox *skybox = nullptr;
    clock_t startTime;

public:
    virtual void Initialize()
    {
        draw_axes = false;
        startTime = clock();
        OpenGLViewer::Initialize();
    }

    virtual void Initialize_Data()
    {
        //// Load all the shaders you need for the scene 
        //// In the function call of Add_Shader_From_File(), we specify three names: 
        //// (1) vertex shader file name
        //// (2) fragment shader file name
        //// (3) shader name used in the shader library
        //// When we bind a shader to an object, we implement it as follows:
        //// object->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("shader_name"));
        //// Here "shader_name" needs to be one of the shader names you created previously with Add_Shader_From_File()

        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/basic.frag", "basic");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/environment.frag", "environment");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/stars.vert", "shaders/stars.frag", "stars");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/alphablend.frag", "blend");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/billboard.vert", "shaders/alphablend.frag", "billboard");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/terrain.vert", "shaders/terrain.frag", "terrain");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/skybox.vert", "shaders/skybox.frag", "skybox");

// My Shaders -- 
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/worley.vert", "shaders/worley.frag", "worley"); // Worley Noise
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/terrainAbs.vert", "shaders/terrainAbs.frag", "terrainAbs"); // Abs Value of Perlin Noise

        //// Load all the textures you need for the scene
        //// In the function call of Add_Shader_From_File(), we specify two names:
        //// (1) the texture's file name
        //// (2) the texture used in the texture library
        //// When we bind a texture to an object, we implement it as follows:
        //// object->Add_Texture("tex_sampler", OpenGLTextureLibrary::Get_Texture("tex_name"));
        //// Here "tex_sampler" is the name of the texture sampler2D you used in your shader, and
        //// "tex_name" needs to be one of the texture names you created previously with Add_Texture_From_File()

        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_color.png", "sphere_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_normal.png", "sphere_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_color.jpg", "bunny_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_normal.png", "bunny_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/window.png", "window_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/buzz_color.png", "buzz_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/star.png", "star_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/waterTexture.png", "waterTexture");
// My Textures --
    // Sun --> Sky Sphere
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/sun_color.png", "sun_color"); // Fully laided out jpg
        //OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/sun_normalV1.png", "sun_normalV1"); // Normal mapping
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/sun_normalV2.png", "sun_normalV2"); // Normal mapping

    // Moon --> Sky Sphere
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon_color.png", "moon_color"); 
        //OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon_normalV1.png", "moon_normalV1");
        //OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon_normalV2.png", "moon_normalV2");
        //OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon_normalll.png", "moon_normalV4"); 
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/moon_normalV3.png", "moon_normalV3"); // Normal mapping

    // Volcano 
    OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/volcano_color.png", "volcano_color"); 
    OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/volcano_normal.png", "volcano_normal"); // Normal mapping


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// L I G H T S
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Add all the lights you need for the scene (no more than 4 lights)
    //// The four parameters are position, ambient, diffuse, and specular.
    //// The lights you declared here will be synchronized to all shaders in uniform lights.
    //// You may access these lights using lt[0].pos, lt[1].amb, lt[1].dif, etc.
    //// You can also create your own lights by directly declaring them in a shader without using Add_Light().
    //// Here we declared three default lights for you. Feel free to add/delete/change them at your will.

            // Light is set to be in same position as the sun
        opengl_window->Add_Light(Vector3f(-0.5, 25, 70), Vector3f(0.1, 0.1, 0.1), Vector3f(1, 1, 1), Vector3f(0.5, 0.5, 0.5)); // Light set to illuminate ocean from front
            // Light to test worley noise --> from front
        opengl_window->Add_Light(Vector3f(-12.5, 17.5, -70.5), Vector3f(0.1, 0.1, 0.1), Vector3f(1, 1, 1), Vector3f(0.5, 0.5, 0.5)); // Light set to illuminate scene from sun's POV (+ coordinates)
            // OG Given
        opengl_window->Add_Light(Vector3f(0, 0, 1), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));
            // OG Given
        opengl_window->Add_Light(Vector3f(-5, 1, 3), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// B A C K G R O U N D 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //// Add the background / environment
        //// Here we provide you with four default options to create the background of your scene:
        //// (1) Gradient color (like A1 and A2; if you want a simple background, use this one)
        //// (2) Programmable Canvas (like A7 and A8; if you consider implementing noise or particles for the background, use this one)
        //// (3) Sky box (cubemap; if you want to load six background images for a skybox, use this one)
        //// (4) Sky sphere (if you want to implement a sky sphere, enlarge the size of the sphere to make it colver the entire scene and update its shaders for texture colors)
        //// By default, Option (2) (Buzz stars) is turned on, and all the other three are commented out.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //// Background Option (1): Gradient color
// 1. CONSTANT MAROON COLOR 
        /*
        {
            auto bg = Add_Interactive_Object<OpenGLBackground>();
            bg->Set_Color(OpenGLColor(0.1f, 0.1f, 0.1f, 1.f), OpenGLColor(0.3f, 0.1f, .1f, 1.f));
            bg->Initialize();
        }
        */ 
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //// Background Option (2): Programmable Canvas
        //// By default, we load a GT buzz + a number of stars
// 2. BUZZ W TWINKLINK STARS
        /*
        {
            bgEffect = Add_Interactive_Object<OpenGLBgEffect>();
            bgEffect->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("stars"));
            bgEffect->Add_Texture("tex_buzz", OpenGLTextureLibrary::Get_Texture("buzz_color")); // bgEffect can also Add_Texture
            bgEffect->Initialize();
        }
        */
        

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //// Background Option (3): Sky box
        //// Here we provide a default implementation of a sky box; customize it for your own sky box
// 3. SKYBOX
    // OG Given
        /* 
        {
            // from https://www.humus.name/index.php?page=Textures
            const std::vector<std::string> cubemap_files{
                "cubemap/posx.jpg",     //// + X
                "cubemap/negx.jpg",     //// - X
                "cubemap/posy.jpg",     //// + Y
                "cubemap/negy.jpg",     //// - Y
                "cubemap/posz.jpg",     //// + Z
                "cubemap/negz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
        }
        */ 
       /*
    // My own scene -> 
        // from https://polyhaven.com/a/belfast_sunset_puresky
            // Cube Map made with help from https://matheowis.github.io/HDRI-to-CubeMap/
       {
            const std::vector<std::string> cubemap_files{
                "cubemapV2/px.jpg",     //// + X
                "cubemapV2/nx.jpg",     //// - X
                "cubemapV2/py.jpg",     //// + Y
                "cubemapV2/ny.jpg",     //// - Y
                "cubemapV2/pz.jpg",     //// + Z
                "cubemapV2/nz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
       }
       */

// SKYBOX
    // My own implementation of a water scene -->
        // sky box images sourced from https://github.com/fegennari/3DWorld/tree/master/textures/skybox/water_scene
            // 4/24/24 -->
        //opengl_window->camera_target = Vector3f(9.5, 9.5, 0);
        float camera_distance=100.f; // From openGLWindow.cpp, openGLWindow.h
        //opengl_window->nearclip=.00001f*camera_distance; // TAs suggest to: change this to scroll closer or farther from sun without it disappearing
        opengl_window->farclip=100.f*camera_distance;

        {
            const std::vector<std::string> cubemap_files{
                "cubemapV4/posx.jpg",     //// + X
                "cubemapV4/negx.jpg",     //// - X
                "cubemapV4/posy.jpg",     //// + Y
                "cubemapV4/negy.jpg",     //// - Y
                "cubemapV4/posz.jpg",     //// + Z
                "cubemapV4/negz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
       }

       
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //// Background Option (4): Sky sphere
        //// Here we provide a default implementation of a textured sphere; customize it for your own sky sphere
    // OG Given --> Earth Sphere 
        /*
        {
            //// create object by reading an obj mesh
            auto sphere = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, -1.5,
                0, 1, 0, -1,
                0, 0, 1, 0.5,
                0, 0, 0, 1;
            sphere->Set_Model_Matrix(t);

            //// set object's material
            sphere->Set_Ka(Vector3f(0.1, 0.1, 0.1));
            sphere->Set_Kd(Vector3f(0.7, 0.7, 0.7));
            sphere->Set_Ks(Vector3f(2, 2, 2));
            sphere->Set_Shininess(128);

            //// bind texture to object
            sphere->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("sphere_color"));
            sphere->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

            //// bind shader to object
            sphere->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        */

// Resource to make normal maps on Photoshop --> https://www.youtube.com/watch?v=ymodojBLCSk
// SUN, sky sphere
    
        {
            //// create object by reading an obj mesh
            auto sun = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << 1.2, 0, 0, -12.5, // moves right on left in x axis // 1.5
                0, 1.2, 0, 17.5, // moves up and down in y axis // 10
                0, 0, 1.2, -70.5, // moves forward and back in z axis; appears larger (closer # to 0) or smaller (farther # from 0) // - 50.5
                0, 0, 0, 1;
            sun->Set_Model_Matrix(t);

            //// set object's material
            sun->Set_Ka(Vector3f(0.11, 0.11, 0.11));
            sun->Set_Kd(Vector3f(0.71, 0.71, 0.71));
            sun->Set_Ks(Vector3f(2, 2, 2));
            sun->Set_Shininess(129);

            //// bind texture to object
            sun->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("sun_color"));
            sun->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sun_normalV2")); // sun_normalV2

            //// bind shader to object
            sun->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        
        

// MOON, sky sphere --> WORKS, BUT DECIDED NOT TO USE IN FINAL IMPLEMENTATION!
        /*
        {
            //// create object by reading an obj mesh
            auto moon = Add_Obj_Mesh_Object("obj/sphere.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, 5.5,
                0, 1, 0, 15,
                0, 0, 1, -70.5,
                0, 0, 0, 1;
            moon->Set_Model_Matrix(t);

            //// set object's material
            moon->Set_Ka(Vector3f(0.1, 0.1, 0.1));
            moon->Set_Kd(Vector3f(0.7, 0.7, 0.7));
            moon->Set_Ks(Vector3f(2, 2, 2));
            moon->Set_Shininess(128);

            //// bind texture to object
            moon->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("moon_color")); //moon_color
            moon->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("moon_normalV3")); //moon_normalV1"

            //// bind shader to object
            moon->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        */
        
        
        
        
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// O B J E C T S 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OG Given --> Bunny
        //// Here we load a bunny object with the basic shader to show how to add an object into the scene
        /*
        {
            //// create object by reading an obj mesh
            auto bunny = Add_Obj_Mesh_Object("obj/bunny.obj");

            //// set object's transform
            Matrix4f t;
            t << 8, 0, 0, 0,
                0, 8, 0, -10,
                0, 0, 8, -250,
                0, 0, 0, 1;
            bunny->Set_Model_Matrix(t);

            //// set object's material
            bunny->Set_Ka(Vector3f(0.1, 0.1, 0.1));
            bunny->Set_Kd(Vector3f(0.7, 0.7, 0.7));
            bunny->Set_Ks(Vector3f(2, 2, 2));
            bunny->Set_Shininess(128);

            //// bind texture to object
            bunny->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("bunny_color"));
            bunny->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("bunny_normal"));

            //// bind shader to object
            bunny->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        */
        
        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. VOLCANO, obj
        
        {
            //// create object by reading an obj mesh
            auto volcano = Add_Obj_Mesh_Object("obj/volcano.obj");

            //// set object's transform
            Matrix4f t, r;
            // float radians = M_PI / -4.0f; // 45 degrees
            // r << 1, 0, 0, 0, // rotate to be flat
            //     0, cos(radians), sin(radians), 0,
            //     0, -sin(radians), cos(radians), 0,
            //     0, 0, 0, 1;

            t << 72, 0, 0, 20,
                0, 380, 0, -31.4, // 144, 192, 240 --> -33, 480
                0, 0, 72, -250,
                0, 0, 0, 1;
            volcano->Set_Model_Matrix(t); // r *

            //// set object's material
            volcano->Set_Ka(Vector3f(0.11, 0.11, 0.11));
            volcano->Set_Kd(Vector3f(0.71, 0.71, 0.71));
            volcano->Set_Ks(Vector3f(2, 2, 2));
            volcano->Set_Shininess(127);

            //// bind texture to object
            volcano->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("volcano_color"));
            //bunny->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("volcano_normal"));

            //// bind shader to object
            volcano->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }
        
        

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// T E R R A I N --> W O R L E Y   N O I S E
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ocean -->
    // PERLIN-WORLEY NOISE
        //// Here we show an example of adding a mesh with noise-terrain (A6)
        
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            float radians = M_PI * 0.5f; // 90 degrees
            float rads = M_PI / -4.0f; // 45 degrees
            //// set object's transform
            Matrix4f r, s, t;
            
                // OG Rotation Matrix Given
            /*
            r << 1, 0, 0, 0, // rotate to be flat
                0, 0.67, 0.67, 0,
                0, -0.67, 0.67, 0,
                0, 0, 0, 1;
            */
            radians = M_PI * 0.498f; // 90 degrees
            r << 1, 0, 0, 0, // rotate to be flat
                0, cos(radians), sin(radians), 0,
                0, -sin(radians), cos(radians), 0,
                0, 0, 0, 1;
            

            s << 450, 0, 0, 0, // scale to cover size of ocean 
                0, 450, 0, 0, // 275 --> 300
                0, 0, 450, 0,
                0, 0, 0, 1;
                
            //t << 1, 0, 0, 0, // translate downwards
            //     0, 1, 0, -5000,
            //     0, 0, 1, -400,
            //     0, 0, 0, 1,
    
            t << 1, 0, 0, -0.5, // moves right on left in x axis // 1.5
                0, 1, 0, -50, // moves up and down in y axis // 10
                0, 0, 1, -670.5, // moves forward and back in z axis; appears larger (closer # to 0) or smaller (farther # from 0) // - 50.5
                0, 0, 0, 1;
            
    // NOTE TO SELF: CHANGE THIS ONE TO MOVE X, Y, Z
            t << 1, 0, 0, -700.5, // moves right on left in x axis // 1.5
                0, 1, 0, -74, // moves up and down in y axis // -93
                0, 0, 1, -70.5, // moves forward and back in z axis; appears larger (closer # to 0) or smaller (farther # from 0) // - 50.5
                0, 0, 0, 1;
            terrain->Set_Model_Matrix(t*r*s);//(t*r*s);
            // terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);


            terrain->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("waterTexture"));


            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("worley"));

        }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// T E S T E R S, for terrain (ocean)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WATER 
// TESTER #1 // 
// Note to self: Smaller on L side to test !!!
        /*
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            float radians = M_PI * 0.5f; // 90 degrees
            float rads = M_PI / -4.0f; // 45 degrees
            //// set object's transform
            Matrix4f r, s, t;
                // OG Rotation Matrix Given
            r << 1, 0, 0, 0, // rotate to be flat
                0, 1, 0.67, 0,
                0, -0.67, 1, 0,
                0, 0, 0, 1;
            
            s << 0.5, 0, 0, 0, // scale to cover size of ocean 
                0, 0.5, 0, 0,
                0, 0, 0.5, 0,
                0, 0, 0, 1;

            t << 1, 0, 0, -4, 
                 0, 1, 0, -3,
                 0, 0, 1, -10.5,
                 0, 0, 0, 1,

            terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);


            terrain->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("waterTexture"));

            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("worley"));
        }
        */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ABS | PERLIN NOISE |

// TESTER #2// 
// Note to self: Bigger on R side to test !!!
        /*
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            float radians = M_PI * 0.5f; // 90 degrees
            float rads = M_PI / -4.0f; // 45 degrees
            //// set object's transform
            Matrix4f r, s, t;
                // OG Rotation Matrix Given
            r << 1, 0, 0, 0, // rotate to be flat
                0, 1, 0.67, 0,
                0, -0.67, 1, 0,
                0, 0, 0, 1;
            
            s << 0.5, 0, 0, 0, // scale to cover size of ocean 
                0, 0.5, 0, 0,
                0, 0, 0.5, 0,
                0, 0, 0, 1;

            t << 1, 0, 0, -1, 
                 0, 1, 0, -3,
                 0, 0, 1, -10.5,
                 0, 0, 0, 1,

            terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);

            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("terrain"));
        }
        */ 
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// R E F L E C T I V E    S U R F A C E S 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    // OG Given   
        //// Here we show an example of shading (ray-tracing) a sphere with environment mapping
            // i.e. REFLECTION OF A SURFACE, like water or a metal 
        
// REFCLTIVE metal sphere
        /*
        {
            //// create object by reading an obj mesh
            auto sphere2 = Add_Obj_Mesh_Object("obj/sphere.obj");       
            //// set object's transform
            Matrix4f t;
            t << .6, 0, 0, 0,
                0, .6, 0, -.5,
                0, 0, .6, 1,
                0, 0, 0, 1;
            sphere2->Set_Model_Matrix(t);

            //// bind shader to object
            sphere2->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("environment")); // bind shader to object
        }
        */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A L P H A    B L E N D I N G
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     // OG Given 
        //// Here we show an example of adding a transparent object with alpha blending
        //// This example will be useful if you implement objects such as tree leaves, grass blades, flower pedals, etc.
        //// Alpha blending will be turned on automatically if your texture has the alpha channel
// red-stained, transparent 4-piece WINDOW 
    // ALPHA BLENDING
        /*
        {
            //// create object by reading an obj mesh
            auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, -0.5,
                0, 1, 0, 0,
                0, 0, 1, 1.5,
                0, 0, 0, 1;
            sqad->Set_Model_Matrix(t);

            //// bind texture to object
            sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("window_color"));

            //// bind shader to object
            sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("blend"));
        }
        */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OG Given   
        //// Here we show an example of adding a billboard particle with a star shape using alpha blending
        //// The billboard is rendered with its texture and is always facing the camera.
        //// This example will be useful if you plan to implement a CPU-based particle system.
// transparent, yellow STAR
    // ALPHA BLENDING
        /*
        {
            //// create object by reading an obj mesh
            auto sqad = Add_Obj_Mesh_Object("obj/sqad.obj");

            //// set object's transform
            Matrix4f t;
            t << 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 2.5,
                 0, 0, 0, 1;
            sqad->Set_Model_Matrix(t);

            //// bind texture to object
            sqad->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("star_color"));

            //// bind shader to object
            sqad->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("billboard"));
        }
        */
        

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //// This for-loop updates the rendering model for each object on the list
        for (auto &mesh_obj : mesh_object_array){
            Set_Polygon_Mode(mesh_obj, PolygonMode::Fill);
            Set_Shading_Mode(mesh_obj, ShadingMode::TexAlpha);
            mesh_obj->Set_Data_Refreshed();
            mesh_obj->Initialize();
        }
        Toggle_Play();
    }

    OpenGLTriangleMesh *Add_Obj_Mesh_Object(std::string obj_file_name)
    {
        auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        Array<std::shared_ptr<TriangleMesh<3>>> meshes;
        // Obj::Read_From_Obj_File(obj_file_name, meshes);
        Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name, meshes);

        mesh_obj->mesh = *meshes[0];
        std::cout << "load tri_mesh from obj file, #vtx: " << mesh_obj->mesh.Vertices().size() << ", #ele: " << mesh_obj->mesh.Elements().size() << std::endl;

        mesh_object_array.push_back(mesh_obj);
        return mesh_obj;
    }

    //// Go to next frame
    virtual void Toggle_Next_Frame()
    {
        for (auto &mesh_obj : mesh_object_array)
            mesh_obj->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);

        if (bgEffect){
            bgEffect->setResolution((float)Win_Width(), (float)Win_Height());
            bgEffect->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
            bgEffect->setFrame(frame++);
        }
        OpenGLViewer::Toggle_Next_Frame();
    }

    virtual void Run()
    {
        //opengl_window->farclip=100.f*10;
        OpenGLViewer::Run();
    }
};

int main(int argc, char *argv[])
{
    MyDriver driver;
    driver.Initialize();
    driver.Run();
}

#endif