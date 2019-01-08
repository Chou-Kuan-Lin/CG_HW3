#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "../GL/glew.h"
#include "../GL/glut.h""
#include "../shader_lib/shader.h"
#include "glm/glm.h"
extern "C"
{
#include "glm_helper.h"
}

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};
typedef struct Vertex Vertex;

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void camera_light_ball_move();
GLuint loadTexture(char* name, GLfloat width, GLfloat height);

namespace
{
	char *obj_file_dir = "../Resources/Ball.obj";
	char *bunny_file_dir = "../Resources/bunny.obj";
	char *teapot_file_dir = "../Resources/teapot.obj";
	char *main_tex_dir = "../Resources/honey_comb_master.ppm";	// 網格紋理

	GLfloat light_rad = 0.1;//radius of the light bulb
	float eyet = 0.0;//theta in degree
	float eyep = 90.0;//phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;
	int mousex = 0;
	int mousey = 0;
}

const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

GLuint mainTexture; // 不要騙我喏要自己Load

GLMmodel *model, *bunnyModel, *teapotModel; //TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)

// camera
float eyex = 0.0;
float eyey = 0.64;
float eyez = 3.0;

GLfloat light_pos[] = { 0.5, 1.0, 1.3 };
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };

#define deltaTime (10) // in ms (1e-3 second)
float time;

GLuint program;	// Shader
GLuint vboName;	// VBO
GLuint fbo;	// frame buffer
GLuint tbo;	// texture buffer
int num;	// model->numtriangles;
float textureRepeat_x = 5, textureRepeat_y = 9;	// texture repeat size
int textureWidth = 512;
int textureHeight = 512;

void Tick(int id)
{
	double d = deltaTime / 1000.0;
	time += d;

	glutPostRedisplay();
	glutTimerFunc(deltaTime, Tick, 0); // 100ms for passTime step size
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("CG_HW3_0756616");	// Student ID
	glutReshapeWindow(512, 512);

	glewInit();

	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutTimerFunc(deltaTime, Tick, 0); //pass Timer function

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);
	model = glmReadOBJ(obj_file_dir);

	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	bunnyModel = glmReadOBJ(bunny_file_dir);

	glmUnitize(bunnyModel);
	glmFacetNormals(bunnyModel);
	glmVertexNormals(bunnyModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(bunnyModel);

	teapotModel = glmReadOBJ(teapot_file_dir);

	glmUnitize(teapotModel);
	glmFacetNormals(teapotModel);
	glmVertexNormals(teapotModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(teapotModel);

	//網格紋理
	mainTexture = loadTexture(main_tex_dir, textureWidth, textureHeight);

	num = model->numtriangles;
	//宣告要使用的模型Mesh
	Vertex *obj = new Vertex[num * 3];
	int cn = 0;	// obj counter
	for (int j = 0;j < num;j++)	// triangle number
		for (int k = 0;k < 3;k++)	// 3 vertices
		{
			//position[3]
			obj[cn].position[0] = model->vertices[model->triangles[j].vindices[k] * 3 + 0];
			obj[cn].position[1] = model->vertices[model->triangles[j].vindices[k] * 3 + 1];
			obj[cn].position[2] = model->vertices[model->triangles[j].vindices[k] * 3 + 2];

			//normal[3]
			obj[cn].normal[0] = model->normals[model->triangles[j].nindices[k] * 3 + 0];
			obj[cn].normal[1] = model->normals[model->triangles[j].nindices[k] * 3 + 1];
			obj[cn].normal[2] = model->normals[model->triangles[j].nindices[k] * 3 + 2];

			//texcoord[2]
			obj[cn].texcoord[0] = model->texcoords[model->triangles[j].tindices[k] * 2 + 0];
			obj[cn].texcoord[1] = model->texcoords[model->triangles[j].tindices[k] * 2 + 1];

			cn++;
		}

	// 連結Shader&VBO
	program = createProgram(createShader("Shaders/shader.vert", "vertex"), createShader("Shaders/shader.frag", "fragment"));

	glGenBuffers(1, &vboName);
	glBindBuffer(GL_ARRAY_BUFFER, vboName);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num * 3 * 8, obj, GL_STATIC_DRAW);	//傳遞數值3+3+2
	glEnableVertexAttribArray(0);	// shader.vert ModelPosition
	glEnableVertexAttribArray(1);	// shader.vert ModelNormal
	glEnableVertexAttribArray(2);	// shader.vert ModelTexcoord
	// 編號、數量、間隔(1=8)、位移
	// 從 main.cpp 傳送 Model Information 給 shader.vert ( & shader.frag )
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// frame buffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// set texture
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, textureWidth, textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// texture into frame buffer (attach)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tbo, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat Matrix[16];	// Matrix buffer
	GLint loc;

	// Projection Matrix & Model Matrix
	{
		glUseProgram(program);
		glGetFloatv(GL_PROJECTION_MATRIX, Matrix);
		loc = glGetUniformLocation(program, "ProjectionMatrix");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Matrix);

		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
		loc = glGetUniformLocation(program, "ModelMatrix");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Matrix);
		glPopMatrix();
		glUseProgram(0);
	}

	// frame buffer ( compute depth )
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	// looping teapot and bunny
	{
		glPushMatrix();
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(-2.1, 3.3 - fmod(time, 3), 1.3);
		glmDraw(teapotModel, GLM_TEXTURE);
		glPopMatrix();

		glPushMatrix();
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(2, 3.3 - fmod(time, 3), 1.4);
		glmDraw(bunnyModel, GLM_TEXTURE);
		glPopMatrix();
	}

	// floor
	{
		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glScalef(30, 1, 30);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glBegin(GL_TRIANGLE_STRIP);

		glColor3f(1, 1, 1);
		glVertex3f(1, -0.032, 1);
		glVertex3f(1, -0.031, -1);
		glVertex3f(-1, -0.032, 1);
		glVertex3f(-1, -0.031, -1);

		glEnd();
		glPopMatrix();
		glEnable(GL_CULL_FACE);
	}

	// screen buffer ( draw )
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// looping teapot and bunny
	{
		glPushMatrix();
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(-2.1, 3.3 - fmod(time, 3), 1.3);
		glmDraw(teapotModel, GLM_TEXTURE);
		glPopMatrix();

		glPushMatrix();
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(2, 3.3 - fmod(time, 3), 1.4);
		glmDraw(bunnyModel, GLM_TEXTURE);
		glPopMatrix();
	}

	// floor
	{
		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glScalef(30, 1, 30);
		glTranslatef(0.0f, 0.0f, 0.0f);
		glBegin(GL_TRIANGLE_STRIP);

		glColor3f(1, 1, 1);
		glVertex3f(1, -0.032, 1);
		glVertex3f(1, -0.031, -1);
		glVertex3f(-1, -0.032, 1);
		glVertex3f(-1, -0.031, -1);

		glEnd();
		glPopMatrix();
		glEnable(GL_CULL_FACE);
	}

	//please try not to modify the following block of code(you can but you are not supposed to)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
			eyex, eyey, eyez,
			eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180),
			eyey + sin(eyet*M_PI / 180),
			eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
			0.0, 1.0, 0.0);
		glPushMatrix();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
	}

	// glmDraw(model, GLM_TEXTURE);	// 用了就GG

	// Model View Matrix
	{
		glUseProgram(program);
		glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
		loc = glGetUniformLocation(program, "ModelViewMatrix");
		glUniformMatrix4fv(loc, 1, GL_FALSE, Matrix);
	}

	// 透明
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthMask(GL_FALSE);

	// Texutre
	{
		glActiveTexture(GL_TEXTURE0 + 0); //GL_TEXTUREi = GL_TEXTURE0 + i
		glBindTexture(GL_TEXTURE_2D, mainTexture);
		loc = glGetUniformLocation(program, "mainTexture");
		glUniform1i(loc, 0);		
		loc = glGetUniformLocation(program, "textureRepeatX");
		glUniform1f(loc, textureRepeat_x);
		loc = glGetUniformLocation(program, "textureRepeatY");
		glUniform1f(loc, textureRepeat_y);
		// Intersect HighLight
		{
			loc = glGetUniformLocation(program, "textureWidth");
			glUniform1f(loc, textureWidth);
			loc = glGetUniformLocation(program, "textureHeight");
			glUniform1f(loc, textureHeight);
			glActiveTexture(GL_TEXTURE1);	// texture buffer
			glBindTexture(GL_TEXTURE_2D, tbo);
			loc = glGetUniformLocation(program, "depthTexture");
			glUniform1i(loc, 1);
		}
	}

	// Pulsing Effect
	loc = glGetUniformLocation(program, "pulsingClock");
	glUniform1f(loc, time);

	// Rim Effect
	GLfloat cameraPosition[3] = { eyex, eyey, eyez };
	loc = glGetUniformLocation(program, "cameraPosition");
	glUniform3fv(loc, 1, cameraPosition);
	
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);

	// 畫到Buff0上
	glDrawArrays(GL_TRIANGLES, 0, num * 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	// 清暫存
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

	glPopMatrix();

	//glDepthMask(GL_TRUE);

	glutSwapBuffers();
	camera_light_ball_move();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	//ESC
		printf("Exit.\n");
		break;
	}
	case 'b'://toggle mode
	{
		printf("No mode can switch.\n");
		break;
	}
	// Camera
	{
	case 'd':
	{
		printf("-----Camera Right-----\n");
		right = true;
		break;
	}
	case 'a':
		printf("-----Camera Left-----\n");
		{
			left = true;
			break;
		}
	case 'w':
	{
		printf("-----Camera Forward-----\n");
		forward = true;
		break;
	}
	case 's':
	{
		printf("-----Camera Backward-----\n");
		backward = true;
		break;
	}
	case 'q':
	{
		printf("-----Camera Up-----\n");
		up = true;
		break;
	}
	case 'e':
	{
		printf("-----Camera Down-----\n");
		down = true;
		break;
	}
	}
	// Light
	{
	case 't':
	{
		printf("-----Light lforward-----\n");
		lforward = true;
		break;
	}
	case 'g':
	{
		printf("-----Light lbackward-----\n");
		lbackward = true;
		break;
	}
	case 'h':
	{
		printf("-----Light lright-----\n");
		lright = true;
		break;
	}
	case 'f':
	{
		printf("-----Light lleft-----\n");
		lleft = true;
		break;
	}
	case 'r':
	{
		printf("-----Light lup-----\n");
		lup = true;
		break;
	}
	case 'y':
	{
		printf("-----Light ldown-----\n");
		ldown = true;
		break;
	}
	}
	// Ball
	{
	case 'i':
	{
		printf("-----Ball bforward-----\n");
		bforward = true;
		break;
	}
	case 'k':
	{
		printf("-----Ball bbackward-----\n");
		bbackward = true;
		break;
	}
	case 'l':
	{
		printf("-----Ball bright-----\n");
		bright = true;
		break;
	}
	case 'j':
	{
		printf("-----Ball bleft-----\n");
		bleft = true;
		break;
	}
	case 'u':
	{
		printf("-----Ball bup-----\n");
		bup = true;
		break;
	}
	case 'o':
	{
		printf("-----Ball bdown-----\n");
		bdown = true;
		break;
	}
	case '7':
	{
		printf("-----Ball bx-----\n");
		bx = true;
		break;
	}
	case '8':
	{
		printf("-----Ball by-----\n");
		by = true;
		break;
	}
	case '9':
	{
		printf("-----Ball bz-----\n");
		bz = true;
		break;
	}
	case '4':
	{
		printf("-----Ball brx-----\n");
		brx = true;
		break;
	}
	case '5':
	{
		printf("-----Ball bry-----\n");
		bry = true;
		break;
	}
	case '6':
	{
		printf("-----Ball brz-----\n");
		brz = true;
		break;
	}
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		printf("-----Move light to front of camera-----\n");
		light_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180);
		light_pos[1] = eyey + sin(eyet*M_PI / 180);
		light_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180);
		break;
	}
	case 'x'://move ball to front of camera
	{
		printf("Move ball to front of camera.\n");
		ball_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet*M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		printf("Reset all.\n");
		light_pos[0] = 1.1;
		light_pos[1] = 1.0;
		light_pos[2] = 1.3;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = 0.0;
		eyey = 0.64;
		eyez = 3.0;
		eyet = 0;
		eyep = 90;
		break;
	}
	// Texture repeat size
	{
	case 'm':
	{
		printf("Texture repeat X Up.\n");
		textureRepeat_x += 0.1;
		break;
	}
	case ',':
	{
		printf("Texture repeat X Down.\n");
		textureRepeat_x -= 0.1;
		break;
	}
	case '.':
	{
		printf("Texture repeat Y Up.\n");
		textureRepeat_y += 0.1;
		break;
	}
	case '/':
	{
		printf("Texture repeat Y Down.\n");
		textureRepeat_y -= 0.1;
		break;
	}
	}
	default:
	{
		break;
	}
	}
}

//no need to modify the following functions
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	if (mleft)
	{
		eyep -= (x - mousex)*0.1;
		eyet -= (y - mousey)*0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}

void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz = 0;
	if (left || right || forward || backward || up || down)
	{
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		eyey += dy * sin(eyet*M_PI / 180);
		eyez += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if (lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		light_pos[1] += dy * sin(eyet*M_PI / 180);
		light_pos[2] += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if (ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		ball_pos[1] += dy * sin(eyet*M_PI / 180);
		ball_pos[2] += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if (bx || by || bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}

void keyboardup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
	{
		right = false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}

void idle(void)
{
	glutPostRedisplay();
}

GLuint loadTexture(char* name, GLfloat width, GLfloat height)
{
	return glmLoadTexture(name, false, true, true, true, &width, &height);
}