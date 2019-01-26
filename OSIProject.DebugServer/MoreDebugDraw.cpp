#include "stdafx.h"

#include <Windows.h>
//#include <d3dx9math.h>
/*#define __RPC__in_xcount
#define __RPC__inout_xcount*/
#include <d3d8.h>
//#include <d3dx8.h>

#include "MoreDebugDraw.h"
#include "MinHook.h"

#include "Native/ScIdentifier.h"
#include "Native/Vector.h"

using namespace LOMNHook::Native;

struct Vector3 {
	float X;
	float Y;
	float Z;

	Vector3() : X(0.0f), Y(0.0f), Z(0.0f) {

	}

	Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {

	}
};

struct Vector4 {
	float X;
	float Y;
	float Z;
	float W;

	Vector4() : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f) {

	}

	Vector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {

	}
};

typedef void ScRenderer;
typedef void GcArea;
typedef D3DMATRIX ScMatrix;
typedef Vector3 ScPoint3d;
typedef Vector3 ScVector;

struct __declspec(align(4)) ScPlane
{
	ScVector Direction;
	float Distance;
};

struct __declspec(align(4)) SxReferenceCountable
{
	void* vtable;
	int count;
};

struct GcTriggerPlane
{
	SxReferenceCountable super;
	ScPlane Plane;
	ScPoint3d points[4];
	float float_72;
	float float_76;
	float float_80;
	float float_84;
	float float_88;
	float float_92;
	float float_96;
	float float_100;
	BYTE byte_104;
	BYTE byte_105;
	BYTE byte_106;
};

struct __declspec(align(4)) _GcTriggerPlaneStruct
{
	ScIdentifier id;
	GcTriggerPlane *trigger_plane;
	BYTE bytes_8[4];
	ScIdentifier id_12;
	ScIdentifier id_16;
	ScIdentifier id_20;
	ScIdentifier id_24;
	ScIdentifier id_28;
};

struct __declspec(align(4)) SxTypedBase
{
	void *get_type_info_ptr;
};

struct __declspec(align(2)) GcCollisionObject
{
	SxReferenceCountable super;
	SxTypedBase typed;
	BYTE byte_12;
	ScPoint3d center_point;
	float top_y;
	float bottom_y;
	ScPoint3d point3d_36;
	float float_48;
	float float_52;
	ScPoint3d point3d_maybe_position_56;
	float float_68;
	float float_72;
	float half_largest_side_length;
	ScPoint3d point3d_80;
	ScPoint3d point3d_maybe_position_92;
	BYTE byte_94;
	BYTE byte_95;
	BYTE byte_96;
};

struct GcBoundaryLine
{
	SxReferenceCountable super;
	ScPoint3d point_1;
	ScPoint3d point_2;
};

struct __declspec(align(4)) GcBoundingPoly
{
	GcCollisionObject super;
	DWORD unused1;
	DWORD unused2;
	Vector<GcBoundaryLine> boundary_vector_1;
	Vector<GcBoundaryLine> boundary_vector_2;
	ScPoint3d point_1;
	ScPoint3d point_2;
	ScPoint3d point_3;
	ScPoint3d point_4;
	ScPoint3d point_5;
	char unk;
};

struct _GcTriggerBox
{
	ScIdentifier id;
	GcBoundingPoly *bounds;
	BYTE b1;
	BYTE b2;
};

typedef void(__stdcall *ScRenderer__DrawLines)(ScPoint3d* points, unsigned int count, const Vector4* color, float unk);

typedef int(__fastcall *GcArea__Render)(GcArea* _this, void* unused);
GcArea__Render tGcArea__Render;

typedef void(__thiscall *GcBoundingPoly__Render)(GcBoundingPoly* _this);

bool DrawTriggerPlanes = false;
bool DrawTriggerBoxes = false;
bool* gTriggerPlanes = &DrawTriggerPlanes;
bool* gTriggerBoxes = &DrawTriggerBoxes;

#if GAME_EDITION == BETA
bool* GcDebugOptions__sWireframe = (bool*)0x705CA8;
bool* gCollisionBoxes = (bool*)0x705CAC;
//ScTriggerManager__ProcessTriggers pScTriggerManager__ProcessTriggers = (ScTriggerManager__ProcessTriggers)0x004C2BF0;
ScRenderer__DrawLines pScRenderer__DrawLines = (ScRenderer__DrawLines)0x00530D50;
GcArea__Render pGcArea__Render = (GcArea__Render)0x00505840;
GcBoundingPoly__Render pGcBoundingPoly__Render = (GcBoundingPoly__Render)0x00431950;
Vector<_GcTriggerPlaneStruct>* pGcCollisionPhysicsGroup__sTriggerPlanes = (Vector<_GcTriggerPlaneStruct>*)0x0083B004;
Vector<_GcTriggerPlaneStruct>* pGcCollisionPhysicsGroup__sLockedOutTriggerPlanes = (Vector<_GcTriggerPlaneStruct>*)0x008390C8;
Vector<_GcTriggerBox>* pGcCollisionPhysicsGroup__sTriggerBoxes = (Vector<_GcTriggerBox>*)0x0083B33C;
Vector<_GcTriggerBox>* pGcCollisionPhysicsGroup__sLockedOutTriggerBoxes = (Vector<_GcTriggerBox>*)0x00839030;
#elif GAME_EDITION == ALPHA
bool* GcDebugOptions__sWireframe = (bool*)0x00610124;
bool* gCollisionBoxes = (bool*)0x0061012C;
// TODO: pScRenderer__DrawLines
#endif

void DebugPrepPoint(ScPoint3d* points, Vector3 point, float size) {
	points[0] = ScPoint3d(point.X + size, point.Y, point.Z);
	points[1] = ScPoint3d(point.X - size, point.Y, point.Z);
	points[2] = ScPoint3d(point.X, point.Y + size, point.Z);
	points[3] = ScPoint3d(point.X, point.Y - size, point.Z);
	points[4] = ScPoint3d(point.X, point.Y, point.Z + size);
	points[5] = ScPoint3d(point.X, point.Y, point.Z - size);
}

void DebugDrawPoint(Vector3 point, Vector4 color, float size) {
	ScPoint3d points[6];
	DebugPrepPoint(points, point, size);
	pScRenderer__DrawLines(points, 6, &color, 3.0f);
}

void DebugDrawTriggerPlane(_GcTriggerPlaneStruct* plane, Vector4 color) {
	ScPoint3d pts[8 * 4 + 4 + 2];
	int k = 0;
	// Corners
	for (int i = 0; i < 4; i++) { // 8 * 4 points
		DebugPrepPoint(&pts[i * 8], plane->trigger_plane->points[i], 20.0f); // 6 points
		k += 6;
		// Border
		pts[k++] = plane->trigger_plane->points[i];
		pts[k++] = plane->trigger_plane->points[(i + 1) % 4];
		//pScRenderer__DrawLines(pts, 2, &color, 3.0f); // 2 points
	}

	// Diagonals // 4 points
	pts[k++] = plane->trigger_plane->points[0];
	pts[k++] = plane->trigger_plane->points[2];
	//pScRenderer__DrawLines(pts, 2, &color, 3.0f);
	pts[k++] = plane->trigger_plane->points[1];
	pts[k++] = plane->trigger_plane->points[3];
	//pScRenderer__DrawLines(pts, 2, &color, 3.0f);

	// Normal // 2 points
	pts[k].X = (plane->trigger_plane->points[0].X + plane->trigger_plane->points[1].X + plane->trigger_plane->points[2].X + plane->trigger_plane->points[3].X) / 4.0f;
	pts[k].Y = (plane->trigger_plane->points[0].Y + plane->trigger_plane->points[1].Y + plane->trigger_plane->points[2].Y + plane->trigger_plane->points[3].Y) / 4.0f;
	pts[k++].Z = (plane->trigger_plane->points[0].Z + plane->trigger_plane->points[1].Z + plane->trigger_plane->points[2].Z + plane->trigger_plane->points[3].Z) / 4.0f;
	pts[k].X = pts[k - 1].X + plane->trigger_plane->Plane.Direction.X * 20.0f;
	pts[k].Y = pts[k - 1].Y + plane->trigger_plane->Plane.Direction.Y * 20.0f;
	pts[k++].Z = pts[k - 1].Z + plane->trigger_plane->Plane.Direction.Z * 20.0f;

	pScRenderer__DrawLines(pts, 8 * 4 + 4 + 2, &color, 1.0f);

	/*DebugDrawPoint(Vector3(plane->trigger_plane->Plane.Direction.X * -plane->trigger_plane->Plane.Distance,
		plane->trigger_plane->Plane.Direction.Y * -plane->trigger_plane->Plane.Distance,
		plane->trigger_plane->Plane.Direction.Z * -plane->trigger_plane->Plane.Distance), Vector4(1.0f, 0.0f, 1.0f, 1.0f), 5.0f);*/
}

void DebugDrawTriggerBox(_GcTriggerBox* box, Vector4 color) {
	//ScPoint3d pts[24];
	//DebugDrawPoint(box->bounds->super.center_point, color, box->bounds->super.half_largest_side_length);
	pGcBoundingPoly__Render(box->bounds);
}

void DebugDraw() {
	// Origin
	if (*gCollisionBoxes) {
		ScPoint3d points[6];
		points[0] = ScPoint3d(0.0f, 0.0f, 0.0f);
		points[1] = ScPoint3d(100.0f, 0.0f, 0.0f);
		points[2] = ScPoint3d(0.0f, 0.0f, 0.0f);
		points[3] = ScPoint3d(0.0f, 100.0f, 0.0f);
		points[4] = ScPoint3d(0.0f, 0.0f, 0.0f);
		points[5] = ScPoint3d(0.0f, 0.0f, 100.0f);
		Vector4 red(1.0f, 0.0f, 0.0f, 1.0f);
		Vector4 green(0.0f, 1.0f, 0.0f, 1.0f);
		Vector4 blue(0.0f, 0.0f, 1.0f, 1.0f);
		pScRenderer__DrawLines(&points[0], 2, &red, 3.0f);
		pScRenderer__DrawLines(&points[2], 2, &green, 3.0f);
		pScRenderer__DrawLines(&points[4], 2, &blue, 3.0f);
	}

	// Trigger planes
	if (DrawTriggerPlanes) {
		for (int i = 0; i < pGcCollisionPhysicsGroup__sTriggerPlanes->Count; i++) {
			DebugDrawTriggerPlane(&pGcCollisionPhysicsGroup__sTriggerPlanes->Data[i], Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		for (int i = 0; i < pGcCollisionPhysicsGroup__sLockedOutTriggerPlanes->Count; i++) {
			DebugDrawTriggerPlane(&pGcCollisionPhysicsGroup__sLockedOutTriggerPlanes->Data[i], Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	// Trigger Boxes
	if (DrawTriggerBoxes) {
		for (int i = 0; i < pGcCollisionPhysicsGroup__sTriggerBoxes->Count; i++) {
			DebugDrawTriggerBox(&pGcCollisionPhysicsGroup__sTriggerBoxes->Data[i], Vector4(0.0f, 0.0f, 1.0f, 1.0f));
		}
		for (int i = 0; i < pGcCollisionPhysicsGroup__sLockedOutTriggerPlanes->Count; i++) {
			DebugDrawTriggerBox(&pGcCollisionPhysicsGroup__sLockedOutTriggerBoxes->Data[i], Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}
}

int __fastcall hGcArea__Render(GcArea* _this, void* unused) {
	int result = tGcArea__Render(_this, unused);
	//if (*gCollisionBoxes)
	DebugDraw();
	return result;
}

void MDDInitialize() {
	//s = MH_CreateHook(pScTriggerManager__ProcessTriggers, &ScTriggerManager::ProcessTriggers, (void**)&tScTriggerManager__ProcessTriggers);
	MH_STATUS s = MH_CreateHook(pGcArea__Render, &hGcArea__Render, (void**)&tGcArea__Render);
}

void MDDShutdown() {

}