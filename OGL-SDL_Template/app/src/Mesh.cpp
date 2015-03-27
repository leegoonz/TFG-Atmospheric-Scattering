
#include "Mesh.h"


Mesh::Mesh() : _vertexCount(0), _texCoordCount(0), _facesCount(0) {}

void Mesh::initOGLData() {

	// Seleccionamos los shaders que queremos cargar
	ShaderInfo shaders1[] = {
		{ GL_VERTEX_SHADER, "../OGL-SDL_Template/app/shaders/scattering.vs.glsl" },
		{ GL_FRAGMENT_SHADER, "../OGL-SDL_Template/app/shaders/scattering.fs.glsl" },
		{ GL_NONE, NULL }
	};

	_render_prog = LoadShaders(shaders1);
	glUseProgram(_render_prog);
	_render_texture_loc = glGetUniformLocation(_render_prog, "texture_diffuse");
	glUniform1i(_render_texture_loc, 0); //Texture unit 0 is for base images.
	_render_projection_matrix_loc = glGetUniformLocation(_render_prog, "projection_matrix");


	_uniform_lightDir = glGetUniformLocation(_render_prog, "lightDir");
	_uniform_cam = glGetUniformLocation(_render_prog, "cam");
	//_uniform_density = glGetUniformLocation(_render_prog, "density");
	_uniform_betaER = glGetUniformLocation(_render_prog, "betaER");
	_uniform_betaEM = glGetUniformLocation(_render_prog, "betaEM");
	_uniform_betaSR = glGetUniformLocation(_render_prog, "betaSR");
	_uniform_betaSM = glGetUniformLocation(_render_prog, "betaSM");
	_uniform_lightSun = glGetUniformLocation(_render_prog, "lightSun");


	// Pedimos un buffer para el element buffer object
	glGenBuffers(1, _ebo);
	// Le hacemos hueco diciendole el tipo de buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo[0]);
	// Lo rellenamos con los indices de los cubos
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _sizeOfFaces, _faces, GL_STATIC_DRAW);

	// Pedimos un array de vertices
	glGenVertexArrays(1, _vao);
	// Le hacemos hueco
	glBindVertexArray(_vao[0]);

	// Pedimos un buffer para el vertex buffer object
	glGenBuffers(1, _vbo);
	// Le hacemos hueco
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	// Le decimos que el hueco tiene que ser de tama�o "tama�o de cube positions"+"tama�o de cube colors"
	glBufferData(GL_ARRAY_BUFFER, _sizeOfVertex + _sizeOfTexCoords, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _sizeOfVertex, _vertices);
	glBufferSubData(GL_ARRAY_BUFFER, _sizeOfVertex, _sizeOfTexCoords, _texCoord);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, (const GLvoid*)(_sizeOfVertex));


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	_texture_id = 0;

	// You should probably use CSurface::OnLoad ... ;)
	//-- and make sure the Surface pointer is good!
	stringstream ss;
	ss << "../OGL-SDL_Template/app/resources/ObjTex/QuantumArid_Diffuse_" << _name[10] << ".jpg";
	_texture = IMG_Load(ss.str().c_str());

	glActiveTexture(GL_TEXTURE0 + 0);
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	glBindSampler(0, GL_LINEAR);

	int mode = GL_RGB;
	if (_texture->format->BytesPerPixel == 4) mode = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, mode, _texture->w, _texture->h, 0, mode, GL_UNSIGNED_BYTE, _texture->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Mesh::cleanup() {
	glDeleteVertexArrays(1, _vao);
	glDeleteBuffers(1, _ebo);
	glDeleteBuffers(1, _vbo);
	glDeleteTextures(1, &_texture_id);
}

void Mesh::draw(vmath::mat4 projection_matrix, vmath::vec4 cameraPos) {

	// Decimos que Shader usar
	glUseProgram(_render_prog);

	// Posicion en el shader, cantidad de matrices, es traspuesta? , matriz a setear.
	// Guardamos las dos matrices en el shader para que dibuje.
	// Al ser uniform el valor, se aprovecha entre los shaders
	glUniformMatrix4fv(_render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

	vmath::vec3 lightDir = vmath::vec3(0.0f, -1.0f, 1.0f);
	vmath::vec3 cam = vmath::vec3(cameraPos[0], cameraPos[1], cameraPos[2]);
	// usamos la unidad de openGL como "un metro"
	// se pude escalar esto correspondientemente para que actue 
	// como si cada unidad de OGL fueran varios metros
	vmath::vec3 betaSR = vmath::vec3(5.8f, 13.5f, 33.1f) * 10e-6f;
	vmath::vec3 betaSM = vmath::vec3(2.0f, 2.0f, 2.0f) * 10e-5f;

	vmath::vec3 betaER = betaSR;
	vmath::vec3 betaEM = betaSM * 1.1f;

	glUniform3fv(_uniform_lightDir, 1, projection_matrix);
	glUniform3fv(_uniform_cam, 1, cam);
	//glUniform3fv(_uniform_density, 1, projection_matrix);
	glUniform3fv(_uniform_betaER, 1, betaER);
	glUniform3fv(_uniform_betaEM, 1, betaEM);
	glUniform3fv(_uniform_betaSR, 1, betaSR);
	glUniform3fv(_uniform_betaSM, 1, betaSM);
	glUniform1f(_uniform_lightSun, 20.0f);


	// Activamos el vertex array Object
	glBindVertexArray(_vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	// Activamos el buffer de indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo[0]);


	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, _texture_id);

	//Dibujamos un strip y otro.
	// Without primitive restart, we need to call two draw commands
	glDrawElements(GL_TRIANGLES, _facesCount * 3, GL_UNSIGNED_INT, NULL);

}

