#include "stdafx.h"

#include "ClassTracker.h"
#include <LOMNAPI.h>
#include <Native/ScIdentifier.h>
#include <Native/_ScBaseString.h>

#include <string>
#include <vector>
#include <map>

using namespace ClassTracker;
using namespace LOMNHook::Native;

// ScIdentifierMap
class ScIdentifierMapBaseBucket {
public:
	LOMNHook::Native::ScIdentifier Key;
	void* Value;
	ScIdentifierMapBaseBucket* Next;
};

class ScIdentifierMapBase {
public:
	void* pfnConstruct;
	void* pfnDestroy;
	ScIdentifierMapBaseBucket** BucketChains;
	DWORD ChainCount;
};

template<typename T>
class ScIdentifierMap : public ScIdentifierMapBase {
public:
	T* GetValue(LOMNHook::Native::ScIdentifier key) {
		int chainIndex = key.AsDWORD % this->ChainCount;
		ScIdentifierMapBaseBucket* bucket = this->BucketChains[chainIndex];

		while (bucket) {
			if (bucket->Key.AsDWORD == key.AsDWORD) {
				return (T*)bucket->Value;
			}

			bucket = bucket->Next;
		}
		return nullptr;
	}
};

class ScTypeInfo {
public:
	void* vtbl;
	DWORD NameHash;
	ScTypeInfo* ParentTypeInfo;
};

typedef DWORD(__cdecl *SrHashString)(const LOMNHook::Native::_ScBaseString*);

#if GAME_EDITION == BETA
unsigned char* IsTypeInfoMapInitialized = (unsigned char*)0x007B5070;
ScIdentifierMap<ScTypeInfo>* TypeInfoMap = (ScIdentifierMap<ScTypeInfo>*)0x007B5074;
SrHashString pSrHashString = (SrHashString)0x004F6AF0;
#endif
/*SrHashString tSrHashString;
std::map<DWORD, const char*> HashedStrings;
DWORD __cdecl hSrHashString(const LOMNHook::Native::_ScBaseString* string) {
	DWORD result = tSrHashString(string);
	if (HashedStrings.find(result) == HashedStrings.end()) {
		HashedStrings.insert(std::pair<const DWORD, const char*>(result, string->Data));
	}
	else {
		OutputDebugStringW(L"[WARNING]: Hash clash!\n");
	}
	return result;
}*/

struct ClassMetadata;

struct ClassComponentMetadata {
	size_t Offset;
	ClassMetadata* ComponentClass;

	ClassComponentMetadata(size_t offset, ClassMetadata* componentClass) : Offset(offset), ComponentClass(componentClass) {

	}
};

std::map<DWORD, ClassMetadata*> ClassMap;
ClassMetadata* SxTypedBaseMetadata = nullptr;
DWORD SxTypedBaseHash;
ClassMetadata* SxStateStampMetadata = nullptr;
DWORD SxStateStampHash;
ClassMetadata* SxReferenceCountableMetadata = nullptr;
DWORD SxReferenceCountableHash;

struct ClassMetadata {
	const char* Name;
	ScTypeInfo* NativeTypeInfo;
	std::vector<ClassComponentMetadata> Components;
	size_t OffsetOfComponent(ClassMetadata* componentClass) {
		for (int i = 0; i < Components.size(); i++)
			if (Components[i].ComponentClass == componentClass)
				return Components[i].Offset;
		return 0xFFFFFFFFUL;
	}
	void* GetComponent(void* instance, ClassMetadata* componentClass) {
		size_t typedBaseOffset = OffsetOfComponent(SxTypedBaseMetadata);
		size_t otherOffset = OffsetOfComponent(componentClass);
		return (void*)((DWORD)instance + otherOffset - typedBaseOffset);
	}
};

ClassMetadata* RegisterClassMetadata(const char* name, size_t sxTypedBaseOffset, size_t sxReferenceCountableOffset, size_t sxStateStampOffset) {
	ClassMetadata* result = new ClassMetadata();
	_ScBaseString str = _ScBaseString(name);
	DWORD hash = pSrHashString(&str);

	if (hash == SxTypedBaseHash)
		SxTypedBaseMetadata = result;
	else if (hash == SxReferenceCountableHash)
		SxReferenceCountableMetadata = result;
	else if (hash == SxStateStampHash)
		SxStateStampMetadata = result;

	ScIdentifier id;
	id.AsDWORD = hash;
	result->NativeTypeInfo = TypeInfoMap->GetValue(id);
	result->Name = name;
	
	if (sxTypedBaseOffset != 0xFFFFFFFF)
		result->Components.push_back(ClassComponentMetadata(sxTypedBaseOffset, SxTypedBaseMetadata));
	if (sxReferenceCountableOffset != 0xFFFFFFFF)
		result->Components.push_back(ClassComponentMetadata(sxReferenceCountableOffset, SxReferenceCountableMetadata));
	if (sxStateStampOffset != 0xFFFFFFFF)
		result->Components.push_back(ClassComponentMetadata(sxStateStampOffset, SxStateStampMetadata));
	ClassMap.insert(std::pair<const DWORD, ClassMetadata*>(hash, result));
	return result;
}

void ClassTracker::InstallHooks() {
	//MH_STATUS s = MH_CreateHook(pSrHashString, hSrHashString, (void**)&tSrHashString);
	MH_EnableHook(MH_ALL_HOOKS);

	_ScBaseString str = _ScBaseString("SxTypedBase");
	SxTypedBaseHash = pSrHashString(&str);
	str = _ScBaseString("SxStateStamp");
	SxStateStampHash = pSrHashString(&str);
	str = _ScBaseString("SxReferenceCountable");
	SxReferenceCountableHash = pSrHashString(&str);

	RegisterClassMetadata("_ScBaseString", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CAbstractPool", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CAbstractPool::Iterator", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CFixedPool", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CFreeBuffer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CMemoryBlockIterator", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("COSPool", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CPoolHall", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CPoolIterator", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CPoolMgr", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CPoolTable", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CRecorder", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CSymbolEngine", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("CVariPool", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("Gc3DSound", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnalogControllerAction", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnalogControlTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnimationEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnimationEventTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnimSprite", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAnimSprite::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcArea", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcArea::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAreaLoader", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcAttackSphere", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAttackSphere::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAttackSpheres", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAttackSpheres::_GcSphereInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcAttackSpheres::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBAGroup", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBaseBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBigSprite", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBigSprite::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBlockWaveReader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBossMeter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBossProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundaryLine", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingCylinder", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingCylinder::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingPoly", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingPoly::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingSphere", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcBoundingSphere::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacter::_GcPiecePair", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacter::AnimationLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacter::RegisterSplits", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacter::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacterAIController", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacterModel", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacterMoveInterpreter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacterMoveList", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCharacterSoundSpawnEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaCharacter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaEditor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaEngine", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaSound", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCinemaText", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionAttackSystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionAttackSystem::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionCylinderSystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionCylinderSystem::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionModel", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionObject", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionObject::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionPhysics", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionPhysicsGroup", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionPolySystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionPolySystem::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionSphereSystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionSphereSystem::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionSystem", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcCollisionSystem::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcCombatHitEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcControllerProcess", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcConversationCharacter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcConversationEditor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcConversationEngine", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcConversationLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcDebugOptions", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcDirectorCallback", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcDragItem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcDragItemCollisionEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEditorButton", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcEditorSlider", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcElementalPower", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEmitter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEmitterLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEnemyProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEventHandler", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEventManager::_GcEventHandlerPair", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcEventTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcFastLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcFilledRectangle", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcFireBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcFloatRange", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcGameEditor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcGlyph", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcHive", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcHive::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcHudFont", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcHudSprite", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcHydraBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcIceBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcIdentifierMap", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcKeyMapper", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcLegoCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcLegoGepetto", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcLine", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcLookatCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMask", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcMaterialTable", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMorphEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMorphModel", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMotionCollisionEventHandler", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMotionSystem", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcMouseDriver", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveableObj", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveableObj::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveInterpreter", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListBasicEntry", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListControllerAction", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListEnum", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListEnum_AnimationLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListEnum_LookUp", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListEnum_RegisterSplits", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMoveListSplitTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMP3Player", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcMudBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcObjAnimLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcObjectMoveInterpreter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcOrthoCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcParticleGroup", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcParticleRender", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcPoly", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcPolyList", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcPolyNC", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcPolyNode", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcPreRenderCinema", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcProjectileCollisionEvent", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRectangle", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRenderNodeGroup", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRenderNodeGroup::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRewards", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRGBARange", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcRockBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcScreenTransition", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcShadow", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcShadowRender", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcShield", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcSoundscapeEditor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSoundTableLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSpecTextureLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSprite", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSprite::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSpriteProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcStringTableLoader", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSurfaceOctNode", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSurfaceOctree", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcSurfaceOctree::_GcOrderedPair", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcTargeteer", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcTargetingReticle", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcTargetObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcTelekineticPower", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcTimer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcToa", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcToaCombatCollisionEventHandler", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcToaProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcTriggerPlane", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcVectorProjectile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcVectorRange", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcViewPort", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("GcVine", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcVisualObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GcWindBoss", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtConvChar", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtConversationCharacter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtConvFrame", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtConvInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtConvMemory", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtEmitterClass", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtEmitterClass::Func", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtEmitterClass::Parm", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtEmitterData", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtFireSpike", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtKeyMapper", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtMap", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtParticle", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtSaverData", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtSaverTime", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("GtTweakInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("Sc2DCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("Sc2DCamera::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScAction", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScActor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScActor::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScAddressMap", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScAnimTexture", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScAutoMutex", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScAutoRefSchizoPtr__ScTrigger__ScTriggerTree_ScTriggerTreeGate", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBKDAnimation", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScBlendedTargetCache", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBlock", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBlockFile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingCube", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingFrustum", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingGeometry2d", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingGeometry3d", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingRect", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBoundingSphere", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScBufferedStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScButtonElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScButtonElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCachedMemoryBuffer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCachedMemoryStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCacheObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCamera::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScChannelBase", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScChannelBase::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveFile", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveFile::ScEntries", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveFile::ScHashEntries", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveFile::ScNameEntries", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveSubFileStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCollectiveVolume", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScComboElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScCompressionCallback", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScContainerTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScContainerTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScContextProvider", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScControlTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScControlTrigger::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScDirectionalLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScDirectionalLight::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawable", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawableContext", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawableMaterial", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawableMaterial::StTextureData", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawableModel", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScDrawableTexture", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScElementThreshold", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFastColor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFastLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFastMatrix", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFastVector", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFileDataCache", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFileManagerStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFileStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFixedString", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFixedStringTemplate", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFont", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScFont::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScGeometryRenderObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScGeometryRenderObject::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScGlobalOSISystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScGLRenderObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScGLRenderObject::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScHUDTransformation", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScHUDTransformation::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifier", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMapBase", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMapsBaseClass", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMapsBaseClass::bucket_type", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMapsBaseClass::const_iterator_base", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMapsBaseClass::iterator_base", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIdentifierMultiMapBase", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScIndexArray", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInitFunc", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputDriver", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputDriver::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputDriverElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputDriverElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputElement", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScInputElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScJoyDriver", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScJoyDriver::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScKeyboardDriver", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScKeyboardDriver::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLight::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScListBase", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScListBase::node_type", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLookAtTargetTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLookAtTargetTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLookAtTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLookAtTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScLZZCompression", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMatrix", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMatrixTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMatrixTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMemoryBuffer", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScMemoryStream", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMeshTargetMorph", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMeshTargetMorph::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMeshTargetMorph::StTargetLink", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMessageHandler", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScModelHierarchy", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScMouseDriver", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMouseDriver::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScMutex", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScNoneCompression", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOneAxisElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOneAxisElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOrientor", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOrientor::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOrthographicCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOrthographicCamera::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIArray", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIDualToken", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIFunctionDescription", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIObject", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIScript", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIStack", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSISystem", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIToken", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIVariant", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScOSIVirtualMachine", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScPerspectiveCamera", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPerspectiveCamera::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPlane", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPlatformScreen", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPoint2d", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPoint3d", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPointInterp", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPointLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPointLight::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPortal", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPortal::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPrinter", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScProcess", 0, 0, 8);
	RegisterClassMetadata("ScProcessManager", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPYRTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScPYRTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScQuat", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScQuaternion", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderer", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderListCallback", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderNode", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderNode::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderNodeGroup", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderNodeGroup::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderObject", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScRenderObject::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScReProcess", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRGBAFloats", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRLECompression", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRotateChannelTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRotateChannelTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRotateTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScRotateTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScaleChannelTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScaleChannelTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScaleTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScaleTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScreen", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScreenData", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScScreenShot", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSequenceElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLKCubicSpline", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLKFile", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSIAnalogControllerAction", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSIAnalogControlTrigger", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSIFileIO", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSIProcess", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSISLKCubicSpline", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScSLOSIStringMapper", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScSOOPTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSOOPTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSpotLight", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSpotLight::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSprite", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScSprite::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScStream", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScSystemVolume", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTCBInterpolator", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTCBQuatInterpolator", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTextObject", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTextObject::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThirdPersonTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThirdPersonTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThreeAxisElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThreeAxisElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThresholdElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScThresholdElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTileFont", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScTimePoint", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTimer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTokenizer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTransformation", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTransformation::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTransformer", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTransformer::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTranslateChannelTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTranslateChannelTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTranslateTransform", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTranslateTransform::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTransparentRenderListCallback", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTrigger", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScTrigger::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTriggerTree", 0, 0, 0xFFFFFFFF);
	RegisterClassMetadata("ScTriggerTree::ScTriggerTreeGate", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTwoAxisElement", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTwoAxisElement::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScVector", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScViewPort", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScVolume", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScWaveSoundRead", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("ScZOrderGreater", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SeActionState", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SeFamilyType", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SsFontCharacter", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("StElementPlatformInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("StFileInitData", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("StImage", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("StRGBABytes", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SxReferenceCountable", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SxStateStamp", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SxTypedBase", 0, 0xFFFFFFFF, 0xFFFFFFFF);
	RegisterClassMetadata("SxTypedBase::ScClassTypeInfo", 0, 0xFFFFFFFF, 0xFFFFFFFF);

	/*OutputDebugStringW(L"ScTypeInfo List:\n");
	for (auto it = ClassMap.begin(); it != ClassMap.end(); it++) {
		OutputDebugStringW(L"  ");
		OutputDebugStringA(it->second->Name);
		OutputDebugStringW(L"\n");
	}*/
	/*for (int i = 0; i < TypeInfoMap->ChainCount; i++) {
		ScIdentifierMapBaseBucket* bucket = TypeInfoMap->BucketChains[i];

		while (bucket) {
			ScTypeInfo* type = (ScTypeInfo*)bucket->Value;

			bucket = bucket->Next;
		}
	}*/
}