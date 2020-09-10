#include "OpenGLWidget.h"
#include <iostream>
#include "MazeWidget.h"
#include <gl\gl.h>
#include <gl\GLU.h>
float ViewMatrix[4][4] = { {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1} }, d;
OpenGLWidget::OpenGLWidget(QWidget *parent) : QGLWidget(parent)
{

	top_z = 1.5f;
	but_z = -1;

	QDir dir("Pic");
	if (dir.exists())
		pic_path = "Pic/";
	else
		pic_path = "../x64/Release/Pic/";
}
void OpenGLWidget::initializeGL()
{
	glClearColor(0, 0, 0, 1);
	glEnable(GL_TEXTURE_2D);
	loadTexture2D(pic_path + "grass.png", grass_ID);
	loadTexture2D(pic_path + "sky.png", sky_ID);
}
void loadMatrix(float m1[][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ViewMatrix[i][j] = m1[i][j];
		}
	}
}
void mult_Matrix(float m1[][4]) {
	float ans[4][4] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				ans[i][j] += ViewMatrix[i][k] * m1[k][j];
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ViewMatrix[i][j] = ans[i][j];
		}
	}
}
void OpenGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (MazeWidget::maze != NULL)
	{
		//View 1
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, MazeWidget::w / 2, MazeWidget::h);
		float maxWH = std::max(MazeWidget::maze->max_xp, MazeWidget::maze->max_yp);
		glOrtho(-0.1, maxWH + 0.1, -0.1, maxWH + 0.1, 0, 10);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		Mini_Map();

		//View 2
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(MazeWidget::w / 2, 0, MazeWidget::w / 2, MazeWidget::h);
		float rad = degree_change(MazeWidget::maze->viewer_fov / 2.0);
		d = -1.0 / tan(rad);
		float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
		float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
		float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];
		rad = degree_change(MazeWidget::maze->viewer_dir);
		float matrix[4][4] = { {sin(rad),0,-cos(rad),0},{0,1,0,0},{-cos(rad),0,-sin(rad),0},{0,0,0,1} };
		float trans[4][4] = { {1,0,0,-MazeWidget::maze->viewer_posn[Maze::X] },
								{0,1,0,-MazeWidget::maze->viewer_posn[Maze::Z] },
								{0,0,1,-MazeWidget::maze->viewer_posn[Maze::Y] },
								{0,0,0,1 } };
		loadMatrix(matrix);
		mult_Matrix(trans);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		Map_3D();
	}
}
void OpenGLWidget::resizeGL(int w, int h)
{
}

//Draw Left Part
void OpenGLWidget::Mini_Map()
{
	glBegin(GL_LINES);

	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

	for (int i = 0; i < (int)MazeWidget::maze->num_edges; i++)
	{
		float edgeStartX = MazeWidget::maze->edges[i]->endpoints[Edge::START]->posn[Vertex::X];
		float edgeStartY = MazeWidget::maze->edges[i]->endpoints[Edge::START]->posn[Vertex::Y];
		float edgeEndX = MazeWidget::maze->edges[i]->endpoints[Edge::END]->posn[Vertex::X];
		float edgeEndY = MazeWidget::maze->edges[i]->endpoints[Edge::END]->posn[Vertex::Y];
		glColor3f(MazeWidget::maze->edges[i]->color[0], MazeWidget::maze->edges[i]->color[1], MazeWidget::maze->edges[i]->color[2]);
		if (MazeWidget::maze->edges[i]->opaque)
		{
			glVertex2f(edgeStartX, edgeStartY);
			glVertex2f(edgeEndX, edgeEndY);
		}
	}

	//draw frustum
	float maxWH = std::max(MazeWidget::maze->max_xp, MazeWidget::maze->max_yp);
	float len = 0.1;
	glColor3f(1, 1, 1);
	glVertex2f(viewerPosX, viewerPosY);
	glVertex2f(viewerPosX + maxWH * len * cos(degree_change(MazeWidget::maze->viewer_dir - MazeWidget::maze->viewer_fov / 2)),
		viewerPosY + maxWH * len * sin(degree_change(MazeWidget::maze->viewer_dir - MazeWidget::maze->viewer_fov / 2)));

	glVertex2f(viewerPosX, viewerPosY);
	glVertex2f(viewerPosX + maxWH * len * cos(degree_change(MazeWidget::maze->viewer_dir + MazeWidget::maze->viewer_fov / 2)),
		viewerPosY + maxWH * len *  sin(degree_change(MazeWidget::maze->viewer_dir + MazeWidget::maze->viewer_fov / 2)));
	glEnd();
}


//**********************************************************************
//
// * Draws the first-person view of the maze.
//   THIS IS THE FUINCTION YOU SHOULD MODIFY.
//
//Note: You must not use any openGL build-in function to set model matrix, view matrix and projection matrix.
//		ex: gluPerspective, gluLookAt, glTraslatef, glRotatef... etc.
//		Otherwise, You will get 0 !
//======================================================================
void mult_Matrix_view(float &x, float &y, float &z) {
	float M[4] = { x,y,z,1 };
	float ans[4] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ans[i] += ViewMatrix[i][j] * M[j];
		}
	}
	x = ans[0]; y = ans[1]; z = ans[2];
}
void Draw(float x[2], float y[2], float z[2], float *a) {
	glBegin(GL_POLYGON);
	glColor3f(a[0], a[1], a[2]);
	float x_wall = x[0], y_wall = z[0], z_wall = y[0], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[0], y_wall = z[1], z_wall = y[0], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[1], y_wall = z[1], z_wall = y[1], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[1], y_wall = z[0], z_wall = y[1], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);
	glEnd();
}
void tellDir(float dir, float &disX, float &disY) {
	if (dir > 360) {
		dir = dir - (int)((int)dir / 360) * 360;
	}
	else if (dir < 0) {
		dir = dir - (int)((int)dir / 360) * 360 + 360;
	}
	if (dir < 180 && dir>0) disY = 1;
	else if (dir > 180 && dir < 360) disY = -1;
	if ((dir < 90 && dir >= 0) || (dir > 270 && dir <= 360)) disX = 1;
	else if ((dir > 90 && dir < 270)) disX = -1;
}
void findRange(float& dif_Angle) {
	if (dif_Angle < -180)  dif_Angle += 360;
	if (dif_Angle > 180)  dif_Angle -= 360;
}
bool canSee(float x[], float y[], float vX, float vY, float &angR, float &angL, float &angleR, float &angleL) {
	float dir = MazeWidget::maze->viewer_dir, disY = 0, disX = 0;
	float disY1 = 0, disX1 = 0, disY2 = 0, disX2 = 0;
	float dirX1 = x[0] - vX, dirX2 = x[1] - vX;
	float dirY1 = y[0] - vY, dirY2 = y[1] - vY;
	dirX1 = (dirX1 > 0 ? 1 : dirX1 < 0 ? -1 : 0);
	dirX2 = (dirX2 > 0 ? 1 : dirX2 < 0 ? -1 : 0);
	dirY1 = (dirY1 > 0 ? 1 : dirY1 < 0 ? -1 : 0);
	dirY2 = (dirY2 > 0 ? 1 : dirY2 < 0 ? -1 : 0);
	tellDir(dir, disX, disY);
	tellDir(angleR, disX1, disY1);
	tellDir(angleL, disX2, disY2);
	if ((dirX1 == disX || dirX1 == disX1 || dirX1 == disX2 || dirX2 == disX || dirX2 == disX1 || dirX2 == disX2)) {
		if ((dirY1 == disY || dirY1 == disY1 || dirY1 == disY2 || dirY2 == disY || dirY2 == disY1 || dirY2 == disY2)) {
			dirX1 = x[0] - vX, dirX2 = x[1] - vX; dirY1 = y[0] - vY, dirY2 = y[1] - vY;
			float m1 = (dirY1 / dirX1), m2 = (dirY2 / dirX2), mL, mR;
			if (x[0] == x[1]) {
				disX = x[0] - vX;
				if (disX > 0) {
					angR = atan2(dirY1, dirX1);
					angL = atan2(dirY2, dirX2);
				}
				else {
					angL = atan2(dirY1, dirX1);
					angR = atan2(dirY2, dirX2);
				}
			}
			else if (y[0] == y[1]) {
				disY = y[0] - vY;
				if (disY > 0) {
					angL = atan2(dirY1, dirX1);
					angR = atan2(dirY2, dirX2);
				}
				else {
					angR = atan2(dirY1, dirX1);
					angL = atan2(dirY2, dirX2);
				}
			}
			float pi = acos(-1);
			angL = angL * 180 / pi;
			angR = angR * 180 / pi;
			if (angL < 0) angL += 360;
			if (angR < 0) angR += 360;
			float difL = angleL - angL, difR = angleR - angR, difL1 = angleL - angR, difR1 = angleR - angL;
			findRange(difL);
			findRange(difL1);
			findRange(difR);
			findRange(difR1);
			if ((difL < 0 && difL1 < 0) || (difR > 0 && difR1 > 0)) return false;
			if (difR > 0) angR = angleR;
			if (difL < 0) angL = angleL;
			return true;
		}
	}
	return false;
}
void Draw_Cell(Cell *c, float angleR, float angleL) {
	c->counter = 1;
	if (c == NULL) return;
	for (int i = 0; i < 4; i++) {
		float x[2] = { c->edges[i]->endpoints[Edge::START]->posn[Vertex::X],c->edges[i]->endpoints[Edge::END]->posn[Vertex::X] };
		float y[2] = { c->edges[i]->endpoints[Edge::START]->posn[Vertex::Y],c->edges[i]->endpoints[Edge::END]->posn[Vertex::Y] };
		float vX = MazeWidget::maze->viewer_posn[Maze::X];
		float vY = MazeWidget::maze->viewer_posn[Maze::Y];
		float *a = c->edges[i]->color, angL, angR;
		if (c->edges[i]->opaque) {
			if (canSee(x, y, vX, vY, angR, angL, angleR, angleL)) {
				if (x[0] == x[1]) {
					float m1 = tan(angL / 180.0f * 3.14159f), m2 = tan(angR / 180.0f * 3.14159f);
					float y1 = m1 * (x[0] - vX) + vY, y2 = m2 * (x[1] - vX) + vY;
					float z[2] = { -1,2 };
					if (y1 > y[1]) y1 = y[1];
					if (y1 < y[0]) y1 = y[0];
					if (y2 > y[1]) y2 = y[1];
					if (y2 < y[0]) y2 = y[0];
					y[0] = y1; y[1] = y2;
					Draw(x, y, z, a);
				}
				else if (y[0] == y[1]) {
					float m1 = tan(angL / 180.0f * 3.14159f), m2 = tan(angR / 180.0f * 3.14159f);
					float x1 = (y[0] - vY) / m1 + vX, x2 = (y[1] - vY) / m2 + vX, max, min;
					float z[2] = { -1,2 };
					if (x1 > x[1]) x1 = x[1];
					if (x1 < x[0]) x1 = x[0];
					if (x2 > x[1]) x2 = x[1];
					if (x2 < x[0]) x2 = x[0];
					x[0] = x1; x[1] = x2;
					Draw(x, y, z, a);
				}
			}
		}
		else {
			if (c->edges[i]->Neighbor(c)->counter != 1) {
				if (canSee(x, y, vX, vY, angR, angL, angleR, angleL))
					Draw_Cell(c->edges[i]->Neighbor(c), angR, angL);
			}
		}
	}
}
void OpenGLWidget::Map_3D()
{
	glLoadIdentity();
	// 畫右邊區塊的所有東西

	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];
	float dir = MazeWidget::maze->viewer_dir;
	if (dir > 360) {
		dir = dir - (int)((int)dir / 360) * 360;
	}
	else if (dir < 0) {
		dir = dir - (int)((int)dir / 360) * 360 + 360;
	}
	MazeWidget::maze->viewer_dir = dir;
	float range1 = dir - (MazeWidget::maze->viewer_fov / 2.0);
	float range2 = dir + (MazeWidget::maze->viewer_fov / 2.0);
	if (range1 < 0) range1 += 360;
	if (range2 > 360) range2 -= 360;
	for (int j = 0; j < (int)MazeWidget::maze->num_cells; j++) {
		Cell *c = NULL;
		bool isSide = MazeWidget::maze->cells[j]->Point_In_Cell(viewerPosX, viewerPosY, viewerPosZ, c);
		if (isSide)
		{
			Draw_Cell(MazeWidget::maze->cells[j], range1, range2);
			break;
		}
	}
	for (int j = 0; j < (int)MazeWidget::maze->num_cells; j++) {
		MazeWidget::maze->cells[j]->counter = 0;
	}

	/*若有興趣的話, 可以為地板或迷宮上貼圖, 此項目不影響評分*/
	glBindTexture(GL_TEXTURE_2D, sky_ID);

	// 畫貼圖 & 算 UV

	glDisable(GL_TEXTURE_2D);
}
void OpenGLWidget::loadTexture2D(QString str, GLuint &textureID)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	QImage img(str);
	QImage opengl_grass = QGLWidget::convertToGLFormat(img);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opengl_grass.width(), opengl_grass.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, opengl_grass.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);
}
float OpenGLWidget::degree_change(float num)
{
	return num / 180.0f * 3.14159f;
}