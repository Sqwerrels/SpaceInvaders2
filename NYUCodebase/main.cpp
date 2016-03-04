#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <string>

#include <fstream>
#include <iostream>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

class Entity {
public:
	float x;
	float y;
	float width;
	float height;
	bool drawBool = true;
	Entity(float X, float Y, float Width, float Height)
	{
		x = X;
		y = Y;
		width = Width;
		height = Height;
	}

};

class Alien : public Entity
{

	float velocity;
	Alien(float X, float Y, float Width, float Height, float Velocity) : Entity(X, Y, Width, Height)
	{
		velocity = Velocity;
	}
};


class SheetSprite {
public:
	SheetSprite();
	SheetSprite(GLuint TextureID, float U, float V, float Width, float Height, float
		Size, float Vertices[]) : textureID(TextureID), u(U), v(V), width(Width), height(Height), size(Size)
	{
		for (int i = 0; i < 12; i++)
		{
			vertices[i] = Vertices[i];

		}
		xpos = Vertices[0];
		ypos = Vertices[1];
	}
	
	void Draw(ShaderProgram* program);
	float size;
	unsigned int textureID;
	float u;
	float xpos;
	float v;
	float ypos;
	float width;
	float height;
	float vertices[12];
};

void SheetSprite::Draw(ShaderProgram* program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	/*
	float vertices[] = {
		-1.0f * size * aspect, -1.0f * size,
		1.0f * size * aspect, 1.0f * size,
		-1.0f * size * aspect, 1.0f * size,
		1.0f * size * aspect, 1.0f * size,
		-1.0f * size * aspect, -1.0f * size,
		1.0f * size * aspect, -1.0f * size };
	*/
	// draw our arrays
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);







}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

template <typename T> std::string tostr(const T& t) {
	std::ostringstream os;
	os << t;
	return os.str();
}

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	SDL_FreeSurface(surface);

	return textureID;

}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	SDL_Event event;
	bool gameStart = false;
	bool done = false;

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");


	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	GLuint fontTexture = LoadTexture("font2.png");


	float cockPit[] = {

		0, -1.5,
		0.25, -1.25,
		0, -1.25,
		0.25, -1.25,
		0, -1.5,
		0.25, -1.5 
	};

	SheetSprite cockpitSprite = SheetSprite(spriteSheetTexture, 586.0f / 1024.0f, 0.f / 1024.0f, 51.0f / 1024.0f, 75.0f / 1024.0f, 0.2f, cockPit);
	Entity cockpitentity = Entity(0.25, -.75, 51.0f, 75.0f);

	float vertices[] = {
		-3.25, 1.05,
		-3.0, 1.3,
		-3.25, 1.3,
		-3.0, 1.3,
		-3.25, 1.05,
		-3.0, 1.05,

	};



	std::vector<Entity> aliens;
	std::vector<SheetSprite> enemies;

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 10; j++)
		{
			SheetSprite enemy1 = SheetSprite(spriteSheetTexture, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.2, vertices);

			Entity a = Entity(vertices[0], vertices[1], 93.0f, 84.0f);
			aliens.push_back(a);
			enemies.push_back(enemy1);
			/*
			vertices[1] += 0.25;
			vertices[3] += 0.25;
			vertices[5] += 0.25;
			vertices[7] += 0.25;
			vertices[9] += 0.25;
			vertices[11] += 0.25;
			*/
			vertices[0] += 0.5;
			vertices[2] += 0.5;
			vertices[4] += 0.5;
			vertices[6] += 0.5;
			vertices[8] += 0.5;
			vertices[10] += 0.5;
		}

		vertices[0] = -3.25;
		vertices[2] = -3.0;
		vertices[4] = -3.25;
		vertices[6] = -3.0;
		vertices[8] = -3.25;
		vertices[10] = -3.05;


		vertices[1] -= 0.5;
		vertices[3] -= 0.5;
		vertices[5] -= 0.5;
		vertices[7] -= 0.5;
		vertices[9] -= 0.5;
		vertices[11] -= 0.5;
	}

	float vertices2[] = {
		-0.25, -.75,
		0, -.5,
		-.25, -.5,
		0, -0.5,
		-.25, -.75,
		0, -.75
	};


	SheetSprite bulletSprite = SheetSprite(spriteSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2, vertices2);
	
	Entity bulletEntity = Entity(0, 0, 9.0f, 54.0f);


	Matrix projectionMatrix;
	Matrix modelMatrixEnemy;
	Matrix modelMatrixCockpit;
	Matrix modelMatrixBullet;
	Matrix modelMatrix;
	Matrix modelMatrixStart;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;


	std::vector<SheetSprite> bullets;
	
	bool goingLeft = false;
	bool drawBullet = false;
	bool isActive = false;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;


		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		float shiftX = 0;
		float shiftY = 0;
		shiftX += 0.5f * elapsed;
		shiftY += 1.0f * elapsed;

		if (!gameStart) {
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			std::string welcome = "Welcome to Space Invaders!";
			DrawText(&program, fontTexture, welcome, 0.2f, 0.02f);
			modelMatrix.setPosition(-2.5, 1, 1);
			
			program.setModelMatrix(modelMatrixStart);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			

			DrawText(&program, fontTexture, "Press Enter to start!", 0.2f, 0.02f);
			modelMatrixStart.setPosition(-2.5, 0, 1);

			if (keys[SDL_SCANCODE_RETURN]){
				gameStart = true;
			}
		}
		else {
		
			
		
			program.setModelMatrix(modelMatrixEnemy);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
			
			for (int i = 0; i < enemies.size(); i++)
			{
				if (aliens[i].drawBool)
					enemies[i].Draw(&program);
			}

			if (!enemies.empty()){

				
				if (keys[SDL_SCANCODE_UP]){
					//modelMatrixCockpit.Translate(0, shiftY, 0);
					std::ofstream out("test.txt");
					out << enemies[0].xpos << std::endl;
				}

				if (aliens.back().x > 3.2f) {
					goingLeft = true;	
					modelMatrixEnemy.Translate(0, -.05f, 0);
					for (int i = 0; i < aliens.size(); i++)
					{
						aliens[i].y -= shiftY;
					}
				}
				if (aliens[0].x < -3.3f)
				{
					goingLeft = false;
					modelMatrixEnemy.Translate(0, -.05f, 0);
					for (int i = 0; i < aliens.size(); i++)
					{
						aliens[i].y -= shiftY;
					}
				}
				if (!goingLeft){

					modelMatrixEnemy.Translate(shiftX, 0, 0);
					for (int i = 0; i < aliens.size(); i++)
					{
						aliens[i].x += shiftX;
					}
				}
				if (goingLeft)
				{
					
					for (int i = 0; i < aliens.size(); i++)
					{
						aliens[i].x -= shiftX;
					}
					modelMatrixEnemy.Translate(-shiftX, 0, 0);


				}
			}
			//mySprite.Draw(&program);
			program.setModelMatrix(modelMatrixCockpit);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);

			
			cockpitSprite.Draw(&program);
					
			if (keys[SDL_SCANCODE_LEFT] && cockpitentity.x > -2.3f){
				modelMatrixCockpit.Translate(-4*shiftX, 0, 0);
				//modelMatrixBullet.Translate(-shiftX, 0, 0);
				cockpitentity.x -= 4*shiftX;
			}

			if (keys[SDL_SCANCODE_RIGHT] && cockpitentity.x < 2.3f){
				modelMatrixCockpit.Translate(4* shiftX, 0, 0);
				//modelMatrixBullet.Translate(shiftX, 0, 0);
				cockpitentity.x += 4*shiftX;
			}


			program.setModelMatrix(modelMatrixBullet);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);

			//SheetSprite bulletSprite = SheetSprite(spriteSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2, vertices);
			if (keys[SDL_SCANCODE_SPACE] && !isActive){
				modelMatrixBullet.setPosition(cockpitentity.x, cockpitentity.y, 1);
				drawBullet = true;
				isActive = true;
			}

			if (bulletEntity.y >= 2)
			{
				drawBullet = false;
				isActive = false;
				bulletEntity.y = cockpitentity.y;
				modelMatrixBullet.setPosition(cockpitentity.x,cockpitentity.y,0);

			}

			if (drawBullet)
			{
				bulletSprite.Draw(&program);
				modelMatrixBullet.Translate(0, shiftY, 0);
				bulletEntity.y += shiftY;
			}

			//THIS IS WHERE MY BULLET COLLISION CODE WOULD BE IF IT WORKED.
			//Im not quite sure where my error lies, and I'm not sure how to fix either, any assistance or aid will
			// (rather late now I guess) but much appreciated. Sorry about not having a proper game to play :/
			/*
			if (isActive)
			{
				for (int i = 0; i < aliens.size(); i++)
				{
					if ((bulletEntity.x >= aliens[i].x )&& (bulletEntity.x <= (aliens[i].x + aliens[i].width)))
						if ((bulletEntity.y >= aliens[i].y) && (bulletEntity.y <= (aliens[i].y + aliens[i].height)))
						{
							isActive = false;
							drawBullet = false;
							bulletEntity.y = cockpitentity.y;
							modelMatrixBullet.setPosition(cockpitentity.x, cockpitentity.y, 0);
							aliens[i].drawBool = false;

						}
				}
			}
			
			*/
			
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
