//referred to https://open.gl/introduction
#define GLEW_STATIC //static linking of glew....no distribution to users!
//headers
#include <Windows.h>
//#define NO_SDL_GLEXT
#include <GL/glew.h>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <SOIL.h>

//shader sources
const GLchar* vertexSource=
	"#version 150 core\n"	
    "in vec2 position;"
	"in vec3 color;"
	"in vec2 texcoord;"
	"out vec3 Color;"
	"out vec2 Texcoord;"
    "void main() {"
	"	Color=color;"
	"	Texcoord = texcoord;"
    "   gl_Position = vec4(position, 0.0, 1.0);"
    "}";
	//without using unifrom trianglecolor...we get white color inside trianngle bcoz its hardcoded. if we want to change the trianglecolor after the shaders have compiled,we need to use "Uniforms"
	//uniforms is a way to pass data to shaders.
	//And Color is added as input to the fragment shader:
const GLchar* fragmentSource =
    "#version 150 core\n"
	"in vec3 Color;"
	"in vec2 Texcoord;"
    "out vec4 outColor;"
	//"uniform vec3 trianglecolor;"
	"uniform sampler2D texKitten;"
	"uniform sampler2D texPuppy;"	
	"uniform float time;"
    "void main() {"
	"   float factor = (sin(time * 3.0) + 1.0) / 2.0;"
	"	if(Texcoord.y < 0.5)"
	"		outColor= texture(texKitten,Texcoord);"
	"	else"
	//"		outColor = texture(texKitten, vec2(Texcoord.x, 1.0 - Texcoord.y));" //reflection of kitten on lower half!
	 "       outColor = texture(texKitten, vec2(Texcoord.x + sin(Texcoord.y * 60.0 + time * 2.0) / 30.0, 1.0 - Texcoord.y)) * vec4(0.7, 0.7, 1.0, 1.0);" //watery look on lower half
	//"    outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord),  factor/2.0 );"
    "}";
	//mix function here is a special GLSL function that linearly interpolates between two variables based on the third parameter.
	//A value of 0.0 will result in the first value, a value of 1.0 will result in the second value and a value in between will result in a mixture of both values. 
	//for color inversion  outColor = vec4(1-Color.x,1-Color.y,1-Color.z, 1.0); 
int main()
{  // CIRCLE GREEN OUTPUT
/*    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }
*/ 
	auto t_start = std::chrono::high_resolution_clock::now();
	sf::ContextSettings settings;
	//settings.antialiasingLevel = 8; //optional with latest version of sfml
	settings.depthBits =24 ;
	settings.stencilBits = 8 ;
	sf::Window window(sf::VideoMode(800,600),"OpenGL",sf::Style::Close,settings);
	
	//initialize glew
	glewExperimental = GL_TRUE; //glew open gl extension wrangler
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{ //PROBLEM SOMETHING IS WRONG
		std::cout << "glewinit falied...aborting" <<err <<" . "<<std::endl;
	}

			//Vertex Array Objects (VAO). VAOs store all of the links between the attributes and your VBOs with raw vertex data.
			//A VAO is created in the same way as a VBO:
	GLuint vao;
	glGenVertexArrays(1, &vao);			//generate 1 vertex array										
	glBindVertexArray(vao);			//to start using it, simply bind it
			// As soon as you've bound a certain VAO, every time you call glVertexAttribPointer, that information will be stored in that VAO. 
			//This makes switching between different vertex data and vertex formats as easy as binding a different VAO! 
			//Just remember that a VAO doesn't store any vertex data by itself, it just references the VBOs you've created and how to retrieve the attribute values from them.
			//Since only calls after binding a VAO stick to it, make sure that you've created and bound the VAO at the start of your program.

			// how to upload (vertex)data to gfx card . soln use VERTEX BUFFER OBJECT. VBO!!
	GLuint vbo;			//To upload the actual data to it you first have to make vbo the active object by calling glBindBuffer:
	glGenBuffers(1,&vbo);		//generate 1 buffer
	GLfloat vertices[] = {
	//position      color			TExture coordinates
    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f,	1.0f, 0.0f, // Top-right
     
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // Bottom-right
    -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f};  // Bottom-left			
			//because there are two sets of same vertices in two trianngles	. we can see that the repetition of vertex data is a waste of memory. Using an element buffer allows you to reuse data:
			//this can be done by using GLuint elements[] = { 0, 1, 2,2, 3, 0 };
	
	glBindBuffer(GL_ARRAY_BUFFER,vbo);		//This statement makes the VBO we just created the active array buffer. Now that it's active we can copy the vertex data to it.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);		//Now that it's active we can copy the vertex data to it.
			//Notice that this function doesn't refer to the id of our VBO, but instead to the active array buffer. The second parameter specifies the size in bytes. 
			//The final parameter is very important and its value depends on the usage of the vertex data. I'll outline the ones related to drawing here:

			//GL_STATIC_DRAW: The vertex data will be uploaded once and drawn many times (e.g. the world).
			//GL_DYNAMIC_DRAW: The vertex data will be changed from time to time, but drawn many times more than that.
			//GL_STREAM_DRAW: The vertex data will change almost every time it's drawn (e.g. user interface).
			//now we have to tell gfx card how to handle the atttributes of data.....this is done by open gl shading language...GLSL!
			//Shaders are written in a C-style language called GLSL (OpenGL Shading Language).

	GLuint ebo;
	glGenBuffers(1,&ebo);		//generate 1 buffer
			//An element array is filled with unsigned integers referring to vertices bound to GL_ARRAY_BUFFER. If we just want to draw them in the order they are in now, it'll look like this:
	        //these are loaded into video memory just like vbo
	GLuint elements[] = { 0, 1, 2,2, 3, 0 };
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);		//this statement makes the EBO the active array buffer. Now that it's active we can copy the elements data to it.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(elements), elements, GL_STATIC_DRAW);

			

			// three shader stages your vertex data will pass through
			// creating a shader object and loading data into it.
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
			//COMpiling vertex shader
	glCompileShader(vertexShader);
			//Checking if a shader compiled successfully
	/* GLint status;
	   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
			//If status is equal to GL_TRUE, then your shader was compiled successfully. 
			//Retrieving the compile log
	    char buffer[512];
	    glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
	*/
			//The fragment shader is compiled in exactly the same way:
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

			//Combining shaders into a program
			//Up until now the vertex and fragment shaders have been two separate objects. 
			//While they've been programmed to work together, they aren't actually connected yet. This connection is made by creating a program out of these two shaders.
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");		//not necessary bcoz here only there is 1 output
			//Since a fragment shader is allowed to write to multiple buffers, you need to explicitly specify which output is written to which buffer. 
			//Use glDrawBuffers when rendering to multiple buffers, because only the first output will be enabled by default.

			//After attaching both the fragment and vertex shaders, the connection is made by linking the program
	glLinkProgram(shaderProgram);
			//To actually start using the shaders in the program, you just have to call:
	glUseProgram(shaderProgram);
			//Just like a vertex buffer, only one program can be active at a time.

			//Making the link between vertex data and attributes
			//Although we have our vertex data and shaders now, OpenGL still doesn't know how the attributes are formatted and ordered. 
			//You first need to retrieve a reference to the position input in the vertex shader:
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position"); 
			//The location is a number depending on the order of the input definitions. The first and only input position in this example will always have location 0.

	glEnableVertexAttribArray(posAttrib);//lastly the vertex attribute array needs to be enabled.

			//With the reference to the input, you can specify how the data for that input is retrieved from the array:
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), 0);
			//The first parameter references the input. The second parameter specifies the number of values for that input, which is the same as the number of components of the vec. 
			//The third parameter specifies the type of each component and 
			//the fourth parameter specifies whether the input values should be normalized between -1.0 and 1.0 (or 0.0 and 1.0 depending on the format) if they aren't floating point numbers.
			//The last two parameters are arguably the most important here as they specify how the attribute is laid out in the vertex array. 
			//The first number specifies the stride, or how many bytes are between each position attribute in the array. The value 0 means that there is no data in between. 
			//This is currently the case as the position of each vertex is immediately followed by the position of the next vertex. 
			//The last parameter specifies the offset, or how many bytes from the start of the array the attribute occurs. Since there are no other attributes, this is 0 as well.
	GLint colAttrib = glGetAttribLocation(shaderProgram,"color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,7*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));		//he fifth parameter is set to 7*sizeof(float) now, 
			//because each vertex consists of 7 floating point attribute values. The offset of 2*sizeof(float) for the color attribute is there because each vertex starts with 2 floating point values for the position that it has to skip over.

			//setting color for whole triangle.
		//GLint uniColor = glGetUniformLocation(shaderProgram, "trianglecolor");	
	//using a texture. you have to add texture attri to vertices.
	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");	//retreive the reference of texcoord from vertex shader.
	glEnableVertexAttribArray(texAttrib);	//vertex atttr array needs to be enabled
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,7*sizeof(GLfloat), (void*)(5*sizeof(GLfloat)));
		// now : providing access to the texture in the fragment shader to sample pixels from it. This is done by adding a uniform of type sampler2D, which will have a default value of 0.
		//This only needs to be changed when access has to be provided to multiple textures

		//load textures
	GLuint textures[2];
	glGenTextures(2, textures);

	int width, height;	//initial declarations
	unsigned char* image;

	glActiveTexture(GL_TEXTURE0);	//important when there are many texture.....tells which texture unit is active
	glBindTexture(GL_TEXTURE_2D, textures[0]);
			//GL_LINEAR  is linear interpolation	
			//SOIL SIMple open gl image library  helps you to load image files directly into texture objects or creates them 4 u.
		image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);	//The texture units of the samplers are set using the glUniform function 
		//it simply accepts an integer specifying the texture unit.

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
			//openGL expects the first pixel to be located in the bottom-left corner, which means that textures will be flipped when loaded with SOIL directly. 	
		 image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
		 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,GL_UNSIGNED_BYTE, image);
		 SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);		

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint uniTime = glGetUniformLocation(shaderProgram, "time");
	bool running = true;
	while(running)
	{
		sf::Event windowEvent;
		while(window.pollEvent(windowEvent))
		{		//process event
			switch(windowEvent.type)
			{
			case sf::Event::Closed:
				running =false;
				break;
			case sf::Event::KeyPressed:
				if(windowEvent.key.code==sf::Keyboard::Escape)
				running =false;
			break;
			}
		}
			// code for what to display
			auto t_now = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
			glUniform1f(uniTime, time);
			//glUniform3f( 0.5 * sin(time * 4.0f) + 1.0f) / 2.0f); to change some entity wrt time
				
			//Clear the screen to black
         glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT);
		 
		 //glDrawArrays(GL_TRIANGLES,0,6);		//The second parameter specifies the number of indices to draw, the third parameter specifies the type of the element data and the last parameter specifies the offset.
		 glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		 window.display();		//TO put THE back buffer content to front buffer so we can see what the output is!!
	}	 
	glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(2, textures);
}
