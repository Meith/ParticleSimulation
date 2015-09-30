#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL_opengl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
	// Sdl initialization
	SDL_Init(SDL_INIT_EVERYTHING);
	
	// Setting opengl version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	
	// Creating Window
	SDL_Window *window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	
	// Setting context of the window
	SDL_GLContext context = SDL_GL_CreateContext(window);
	
	// Glew initialization
	glewExperimental = GL_TRUE;
	glewInit();
	
	// Print opengl version
	printf("Using opengl version %s.\n", glGetString(GL_VERSION));
	printf("Using glsl version %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	// Shader source
	const GLchar *vert_source = 
		"#version 430\n"
		"layout (location = 0) in vec4 VertexPosition;"
		"out vec3 Position;"
		"uniform vec3 trans;"
		"uniform vec3 rot;"
		"uniform vec3 scale;"
		"uniform vec3 eye;"
		"uniform vec3 up;"
		"uniform vec3 look;"
		"uniform vec4 nfwh;"
		"void main()"
		"{"
		// Translate
		"mat4 trans_mat = mat4(\n"
		"1.0,		0.0,		0.0,		0.0,\n"
		"0.0,		1.0,		0.0,		0.0,\n"
		"0.0,		0.0,		1.0,		0.0,\n"
		"trans.x,	trans.y,	trans.z,	1.0\n"
		");"
		// Rotate
		"vec3 theta = radians(rot);"
		"vec3 c = cos(theta);"
		"vec3 s = sin(theta);"
		"mat4 rot_x = mat4(\n"
		"1.0,	0.0,	0.0,	0.0,\n"
		"0.0,	c.x,	s.x,	0.0,\n"
		"0.0,	-s.x,	c.x,	0.0,\n"
		"0.0,	0.0,	0.0,	1.0\n"
		");"
		"mat4 rot_y = mat4(\n"
		"c.y,	0.0,	-s.y,	0.0,\n"
		"0.0,	1.0,	0.0,	0.0,\n"
		"s.y,	0.0,	c.y,	0.0,\n"
		"0.0,	0.0,	0.0,	1.0\n"
		");"
		"mat4 rot_z = mat4(\n"
		"c.z,	-s.z,	0.0,	0.0,\n"
		"s.z,	c.z,	0.0,	0.0,\n"
		"0.0,	0.0,	1.0,	0.0,\n"
		"0.0,	0.0,	0.0,	1.0\n"
		");"
		// Scale
		"mat4 scale_mat = mat4(\n"
		"scale.x,	0.0,		0.0,		0.0,\n"
		"0.0,		scale.y,	0.0,		0.0,\n"
		"0.0,		0.0,		scale.z,	0.0,\n"
		"0.0,		0.0,		0.0,		1.0\n"	
		");"
		// Camera
		"vec3 n = normalize(eye - look);"
		"vec3 u = normalize(cross(up, n));"
		"vec3 v = cross(n, u);"
		"mat4 ViewMatrix = mat4(\n"
		"u.x,			v.x,			n.x, 			0.0,\n"
		"u.y,			v.y,			n.y,			0.0,\n"
		"u.z,			v.z,			n.z,			0.0,\n"
		"dot(-1 * u, eye),	dot(-1 * v, eye),	dot(-1 * n, eye),	1.0\n"
		");"
		// Projection
		"mat4 ProjectionMatrix = mat4(\n"
		"2 * nfwh.x / nfwh.z,	0.0,			0.0,						0.0,\n"	
		"0.0,			2 * nfwh.x / nfwh.w,	0.0,						0.0,\n"
		"0.0,			0.0,			-1 * (nfwh.y + nfwh.x) / (nfwh.y - nfwh.x),	-1.0,\n"
		"0.0,			0.0,			-2 * (nfwh.y * nfwh.x) / (nfwh.y - nfwh.x),	0.0\n"
		");"
		"	mat4 ModelMatrix = trans_mat * rot_z * rot_y * rot_x * scale_mat;"
		"	Position = (ViewMatrix * ModelMatrix * VertexPosition).xyz;"
		"	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * VertexPosition;"
		"}";
	
	const GLchar *frag_source = 
		"#version 430\n"
		"in vec3 Position;"
		"uniform vec4 Color;"
		"layout (location = 0) out vec4 FragColor;"
		"void main()"
		"{"
		"	FragColor = Color;"
		"}";
	
	const GLchar *comp_source = 
		"#version 430\n"
		"layout (local_size_x = 1000) in;"
		"uniform float Gravity1 = 1000.0;"
		"uniform vec3 BlackHolesPos1 = vec3(5, 0, 0);"
		"uniform float Gravity2 = 1000.0;"
		"uniform vec3 BlackHolesPos2 = vec3(-5, 0, 0);"
		"uniform float ParticleMass = 0.1;"
		"uniform float ParticleInvMass = 1.0 / 0.1;"
		"uniform float DeltaT = 0.0005;"
		"uniform float MaxDist = 45.0f;"
		"layout(std430, binding = 0) buffer Pos\n"
		"{"
		"	vec4 Position[];"
		"};"
		"layout(std430, binding = 1) buffer Vel\n"
		"{"
		"	vec4 Velocity[];"
		"};"
		"void main()"
		"{"
		"	uint idx = gl_GlobalInvocationID.x;"
		"	vec3 p = Position[idx].xyz;"
		"	vec3 d = BlackHolesPos1 - p;"
		"	float dist = length(d);"
		"	vec3 force = (Gravity1 / dist) * normalize(d);"
		"	d = BlackHolesPos2 - p;"
		"	dist = length(d);"
		"	force += (Gravity2 / dist) * normalize(d);"
		"	if(dist > MaxDist)"
		"	{"
		"		Position[idx] = vec4(0, 0, 0, 1);"
		"	}"
		"	else"
		"	{"
		"		vec3 a = force * ParticleInvMass;"
		"		Position[idx] = vec4(p + Velocity[idx].xyz * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);"
		"		Velocity[idx] = vec4(Velocity[idx].xyz + a * DeltaT, 0.0);"
		"	}"
		"}";
	
	// Compiling vertex shader
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vert_source, NULL);
	glCompileShader(vert_shader);
	
	// Check compilation for errors in vertex shader
	GLint status;
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &status);
	printf("Vertex Shader compile status %d\n", status);
	
	char error_log[512];
	glGetShaderInfoLog(vert_shader, 512, NULL, error_log);
	printf("Vertex Shader error log: %s\n", error_log);
	
	// Compiling fragment shader
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_source, NULL);
	glCompileShader(frag_shader);
	
	// Check for compilation errors in fragment shader
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);
	printf("Fragment Shader compile status %d\n", status);
	
	glGetShaderInfoLog(frag_shader, 512, NULL, error_log);
	printf("Fragment Shader error log: %s\n", error_log);
	
	// Convert shader compiled objects into executable program
	GLuint render_prog = glCreateProgram();
	glAttachShader(render_prog, vert_shader);
	glAttachShader(render_prog, frag_shader);

	glDeleteShader(frag_shader);
    	glDeleteShader(vert_shader);
    	
    	// Compiling compute shader
    	GLuint comp_shader = glCreateShader(GL_COMPUTE_SHADER);
    	glShaderSource(comp_shader, 1, &comp_source, NULL);
    	glCompileShader(comp_shader);
    	
    	// Check for compilation erros in compute shader
	glGetShaderiv(comp_shader, GL_COMPILE_STATUS, &status);
	printf("Compute Shader compile status %d\n", status);
	
	glGetShaderInfoLog(comp_shader, 512, NULL, error_log);
	printf("Compute Shader error log: %s\n", error_log);
	
	// Convert into program
	GLuint comp_prog = glCreateProgram();
	glAttachShader(comp_prog, comp_shader);
	
	glDeleteShader(comp_shader); 
	
	// Link shader program which to be used
	glLinkProgram(render_prog);
	glLinkProgram(comp_prog);
	
	// variables and attributes
	float nParticles = 100;
	float totalParticles = 1000000;
	float bh1[] = {5.0f, 0.0f, 0.0f, 1.0f};
	float bh2[] = {-5.0f, 0.0f, 0.0f, 1.0f};
	
	// initial positions of the particles
	int bufSize = totalParticles * 4 * sizeof(float);
	float *initPos = (float *)malloc(bufSize);
	float *initVel = (float *)calloc(totalParticles, sizeof(float));
	
	float p[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	float dx = 2.0f / (nParticles - 1);
	float dy = dx;
	float dz = dx;
	
	int i, j, k, particleCount = 0;
	for(i = 0; i < nParticles; ++i)
	{
		for(j = 0; j < nParticles; ++j)
		{
			for(k = 0; k < nParticles; ++k)
			{
				p[0] = dx * i;
				p[1] = dy * j;
				p[2] = dz * k;
				initPos[particleCount++] = p[0];
				initPos[particleCount++] = p[1];
				initPos[particleCount++] = p[2];
				initPos[particleCount++] = p[3];
			}
		}
	}
	
	// Opengl buffers for position and velocity
	GLuint bufs[2];
	glGenBuffers(2, bufs);
	GLuint posBuf = bufs[0];
	uint velBuf = bufs[1];
	
	// The buffers for positions
  	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf);
  	glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_DRAW);

  	// Velocities
  	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velBuf);
  	glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initVel[0], GL_DYNAMIC_COPY);

  	// Set up the VAO
  	GLuint particlesVao;
  	glGenVertexArrays(1, &particlesVao);
  	glBindVertexArray(particlesVao);

  	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
  	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  	glEnableVertexAttribArray(0);

  	glBindVertexArray(0);

  	// Set up a buffer and a VAO for drawing the attractors (the "black holes")
	GLuint bhBuf;
  	glGenBuffers(1, &bhBuf);
  	glBindBuffer(GL_ARRAY_BUFFER, bhBuf);
  	GLfloat data[] = { bh1[0], bh1[1], bh1[2], bh1[3], bh2[0], bh2[1], bh2[2], bh2[3] };
  	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);

	GLuint bhVao;
  	glGenVertexArrays(1, &bhVao);
  	glBindVertexArray(bhVao);

  	glBindBuffer(GL_ARRAY_BUFFER, bhBuf);
  	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  	glEnableVertexAttribArray(0);

  	glBindVertexArray(0);

	float trans[3] = {0.0f, 0.0f, 0.0f};
	float rot[3] = {0.0f, 0.0f, 0.0f};
	float scale[3] = {1.0f, 1.0f, 1.0f};

	float eye[3] = {0.0f, 0.0f, 30.0f};
	float up[3] = {0.0f, 1.0f, 0.0f};
	float look[3] = {0.0f, 0.0f, 0.0f};

	float nfwh[4] = {1.0f, 100.0f, 1.0f, 1.0f};

	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	float color1[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	SDL_Event window_event;
	while(1)
	{
		if(SDL_PollEvent(&window_event))
		{
			if(window_event.type == SDL_QUIT)
			{
				break;
			}
		}
		
		// Clear screen to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		glUseProgram(comp_prog);
		{
			glUniform3fv(glGetUniformLocation(comp_prog, "BlackHolesPos1"), 1, bh1);
			glUniform3fv(glGetUniformLocation(comp_prog, "BlackHolesPos2"), 1, bh2);	
			glDispatchCompute(totalParticles / 1000, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}	
		
		glUseProgram(render_prog);
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUniform3fv(glGetUniformLocation(render_prog, "trans"), 1, trans);
			glUniform3fv(glGetUniformLocation(render_prog, "rot"), 1, rot);
			rot[2] += 1.0f;
			rot[0] -= 1.0f;
			glUniform3fv(glGetUniformLocation(render_prog, "scale"), 1, scale);
			glUniform3fv(glGetUniformLocation(render_prog, "eye"), 1, eye);
			glUniform3fv(glGetUniformLocation(render_prog, "up"), 1, up);
			glUniform3fv(glGetUniformLocation(render_prog, "look"), 1, look);
			glUniform4fv(glGetUniformLocation(render_prog, "nfwh"), 1, nfwh);
			glPointSize(1.0f);
			glUniform4fv(glGetUniformLocation(render_prog, "Color"), 1, color);
			
			glBindVertexArray(particlesVao);
			{
				glDrawArrays(GL_POINTS, 0, totalParticles);
			}
			glBindVertexArray(0);
			
			glUniform4fv(glGetUniformLocation(render_prog, "Color"), 1, color1);
			glPointSize(10.0f);
			glBindVertexArray(bhVao);
			{
				glDrawArrays(GL_POINTS, 0, 2);
			}
			glBindVertexArray(0);
		}
			
		SDL_GL_SwapWindow(window);
	}
	
	glDeleteProgram(render_prog);
	glDeleteProgram(comp_prog);

	SDL_GL_DeleteContext(context);
	
	SDL_DestroyWindow(window);
		
	SDL_Quit();
	
	return 0;
}
