#include "pch.h"

#include "./G-Logic/GLCharDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WORD school2index ( const EMSCHOOLFLAG emSchool )
{
	switch ( emSchool )
	{
	case GLSCHOOL_00:	return 0;
	case GLSCHOOL_01:	return 1;
	case GLSCHOOL_02:	return 2;
	};

	return 0;
}

EMSCHOOLFLAG index2school ( const WORD wSchool )
{
	switch ( wSchool )
	{
	case 0:	return GLSCHOOL_00;
	case 1:	return GLSCHOOL_01;
	case 2:	return GLSCHOOL_02;
	};

	return GLSCHOOL_00;
}

EMCHARINDEX CharClassToIndex ( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_FIGHTER_M:		return GLCI_FIGHTER_M;
	case GLCC_ARMS_M:			return GLCI_ARMS_M;
	case GLCC_ARCHER_W:		return GLCI_ARCHER_W;
	case GLCC_SPIRIT_W:		return GLCI_SPIRIT_W;
	case GLCC_EXTREME_M:	return GLCI_EXTREME_M;
	case GLCC_EXTREME_W:	return GLCI_EXTREME_W;
	
	case GLCC_FIGHTER_W:	return GLCI_FIGHTER_W;
	case GLCC_ARMS_W:		return GLCI_ARMS_W;
	case GLCC_ARCHER_M:		return GLCI_ARCHER_M;
	case GLCC_SPIRIT_M:		return GLCI_SPIRIT_M;
	};

	GASSERT(0&&"잘못된 케릭터 클래스입니다.");
	return GLCI_FIGHTER_M;
}

EMCHARCLASS CharIndexToClass ( const EMCHARINDEX emIndex )
{
	switch (emIndex)
	{
	case GLCI_FIGHTER_M:	return GLCC_FIGHTER_M;
	case GLCI_ARMS_M:		return GLCC_ARMS_M;
	case GLCI_ARCHER_W:		return GLCC_ARCHER_W;
	case GLCI_SPIRIT_W:		return GLCC_SPIRIT_W;

	case GLCI_EXTREME_M:	return GLCC_EXTREME_M;
	case GLCI_EXTREME_W:	return GLCC_EXTREME_W;

	case GLCI_FIGHTER_W:	return GLCC_FIGHTER_W;
	case GLCI_ARMS_W:		return GLCC_ARMS_W;
	case GLCI_ARCHER_M:		return GLCC_ARCHER_M;
	case GLCI_SPIRIT_M:		return GLCC_SPIRIT_M;
	};

	GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
	return GLCC_FIGHTER_M;
}


EMCHARCLASS CharClassToSex ( const EMCHARCLASS emClass )
{
	switch (emClass)
	{
	case GLCC_FIGHTER_M:		return GLCC_FIGHTER_W;
	case GLCC_ARMS_M:			return GLCC_ARMS_W;
	case GLCC_ARCHER_W:		return GLCC_ARCHER_M;
	case GLCC_SPIRIT_W:		return GLCC_SPIRIT_M;
	case GLCC_EXTREME_M:	return GLCC_EXTREME_W;
	case GLCC_EXTREME_W:	return GLCC_EXTREME_M;
	
	case GLCC_FIGHTER_W:	return GLCC_FIGHTER_M;
	case GLCC_ARMS_W:		return GLCC_ARMS_M;
	case GLCC_ARCHER_M:		return GLCC_ARCHER_W;
	case GLCC_SPIRIT_M:		return GLCC_SPIRIT_W;
	};

	GASSERT(0&&"잘못된 케릭터 클래스입니다.");
	return GLCC_FIGHTER_M;
}

EMCHARINDEX CharIndexToSex ( const EMCHARINDEX emIndex )
{
	switch (emIndex)
	{
	case GLCI_FIGHTER_M:	return GLCI_FIGHTER_W;
	case GLCI_ARMS_M:		return GLCI_ARMS_W;
	case GLCI_ARCHER_W:		return GLCI_ARCHER_M;
	case GLCI_SPIRIT_W:		return GLCI_SPIRIT_M;

	case GLCI_EXTREME_M:	return GLCI_EXTREME_W;
	case GLCI_EXTREME_W:	return GLCI_EXTREME_M;

	case GLCI_FIGHTER_W:	return GLCI_SPIRIT_W;
	case GLCI_ARMS_W:		return GLCI_ARCHER_W;
	case GLCI_ARCHER_M:		return GLCI_ARMS_M;
	case GLCI_SPIRIT_M:		return GLCI_FIGHTER_M;
	};

	GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
	return GLCI_FIGHTER_M;
}

VOID GetCharSkillClassIndex( const EMCHARCLASS emClass, WORD & wBeginIndex, WORD & wEndIndex )
{
	switch ( emClass )
	{
	case GLCC_FIGHTER_M:
	case GLCC_FIGHTER_W:
		{
			wBeginIndex = EMSKILL_FIGHTER_01;
			wEndIndex = EMSKILL_FIGHTER_04;
		}
		break;

	case GLCC_ARMS_M:
	case GLCC_ARMS_W:
		{
			wBeginIndex = EMSKILL_ARMS_01;
			wEndIndex = EMSKILL_ARMS_04;
		}
		break;

	case GLCC_ARCHER_M:
	case GLCC_ARCHER_W:
		{
			wBeginIndex = EMSKILL_ARCHER_01;
			wEndIndex = EMSKILL_ARCHER_04;
		}
		break;

	case GLCC_SPIRIT_M:
	case GLCC_SPIRIT_W:
		{
			wBeginIndex = EMSKILL_SPIRIT_01;
			wEndIndex = EMSKILL_SPIRIT_04;
		}
		break;	

	case GLCC_EXTREME_M:
	case GLCC_EXTREME_W:
		{
			wBeginIndex = EMSKILL_EXTREME_01;
			wEndIndex = EMSKILL_EXTREME_04;
		}
		break;

	default:
		GASSERT(0&&"잘못된 케릭터 인덱스입니다.");
		break;
	};
}

namespace COMMENT
{
	std::string ATIONTYPE[GLAT_SIZE] =
	{
		"GLAT_IDLE",
		"GLAT_MOVE",
		"GLAT_ATTACK",
		"GLAT_SKILL",
		"GLAT_SHOCK",
		"GLAT_PUSHPULL",

		"GLAT_TALK",
		"GLAT_CONFT_END",

		"GLAT_FALLING",
		"GLAT_DIE",
	};

	std::string TRIBE[TRIBE_NSIZE] = 
	{
		"Human",
		"Monster",
		"Angel",
		"Devil",
		"Fairy",
		"Genie",
		"Dragon"
	};

	std::string MOVETYPE[MOVETYPE_NSIZE] = 
	{
		"Ground movement",
		"Air Moverment",
		"Aqua movement"
	};

	std::string ELEMENT[EMELEMENT_MAXNUM2] = 
	{
		"Vital Energy",
		"Fire",
		"Ice",
		"Eletrical energy",
		"Poison",
		"Petrified",
		"Confusion",
		"Fainting",
		"Malediction",
		"All belong to item"
	};

	std::string BRIGHT[BRIGHT_SIZE] =
	{
		"Good",
		"Evil",
		"(Neutral)"
	};

	std::string CHARCLASS[GLCI_NUM_NEWSEX] =
	{
		"격투부남",	//"격투부",
		"검도부남",	//"검도부",
		"양궁부여",	//"양궁부",
		"기예부여",	//"기예부",
		"극강부남",	//"극강부남",
		"극강부여",	//"극강부여",
		"격투부여",	//"격투부",
		"검도부여",	//"검도부",
		"양궁부남",	//"양궁부",
		"기예부남",	//"기예부",
	};

	std::string EMSCHOOL[GLSCHOOL_NUM] =
	{
		"school 1",
		"school 2",
		"school 3",
	};

	std::string CHARCOMMENT[GLCI_NUM_NEWSEX] = 
	{
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		""
	};

	std::string BLOW[EMBLOW_SIZE] =
	{
		"Without",
		"Paralysis",
		"Faint",
		"Petrochemical",
		"Burn",
		"Freeze",

		"Chaos",
		"Poison",
		"Curse",
	};

	std::string BLOW_VAR1[EMBLOW_SIZE] =
	{
		/*
		"없음",
		"이속변화율",	//"마비"
		"없음",			//"기절"
		"이속변화율",	//"석화"
		"없음",			//"화염"
		"이속변화율",	//"냉동"

		"없음",			//"착란"
		"없음",			//"중독"
		"없음",			//"저주"
		*/

		"No",
		"Movement Speed Eate",
		"No",
		"Movement Speed Rate",
		"No",
		"Movement Speed Rate",
		"No",
		"No",
		"No",
	};

	float BLOW_VAR1_SCALE[EMBLOW_SIZE] =
	{
		1.0f,
		100.0f,			//"마비"
		1.0f,			//"기절"
		100.0f,			//"석화"
		1.0f,			//"화염"
		100.0f,			//"냉동"

		1.0f,			//"착란"
		1.0f,			//"중독"
		1.0f,			//"저주"
	};

	std::string BLOW_VAR2[EMBLOW_SIZE] =
	{
		/*
		"없음",
		"딜래이변화율",		//"마비"
		"없음",				//"기절"
		"없음",				//"석화"
		"지속타격치",		//"화염"
		"타격가산율",		//"냉동"

		"없음",				//"착란"
		"지속타격치",		//"중독"
		"지속타격치",		//"저주"
		*/

		"No",
		"Delay Rate",
		"No",
		"No",
		"Continue Damage",
		"Attack Add Rate",
		"No",
		"Continue Damage",
		"Continue Damage",
	};

	float BLOW_VAR2_SCALE[EMBLOW_SIZE] =
	{
		1.0f,
		100.0f,				//"마비"
		1.0f,				//"기절"
		1.0f,				//"석화"
		1.0f,				//"화염"
		100.0f,				//"냉동"

		1.0f,				//"착란"
		1.0f,				//"중독"
		1.0f,				//"저주"
	};

	std::string IMPACT_ADDON[EIMPACTA_SIZE] =
	{
		/*
		"없음",
		"명중율 변화량",
		"회피율 변화량",
	
		"공격치 변화량",
		"방어치 변화량",

		"HP 변화율",
		"MP 변화율",
		"SP 변화율",
		"HP+MP+SP 변화율",

		"공격치 변화율",
		"방어치 변화율",

		"격투치",
		"사격치",
		"기력치",

		"HP 증폭율",
		"MP 증폭율",
		"SP 증폭율",
		"저항치 변화",
		*/

		"None",
		"Hit Percentage Transformation Rate",
		"Avoid Change Rate",
		"Attack Change Amount",
		"Defend Change Amount",
		"HP Change Percentage",
		"MP Change Percentage",
		"SP Change Percentage",
		"HP+MP+SP Change Percentage",
		"Attack Change Percentage",
		"Defend Change Percentage",
		"CloseAttack Rate",
		"Shooting Rate",
		"Energy",
		"HP increasing Percentage",
		"MP Increasing Percentage",
		"SP Increasing Percentage",
		"Change Resistance",
	};

	float IMPACT_ADDON_SCALE[EIMPACTA_SIZE] =
	{
		1.0f,
		1.0f,
		1.0f,
	
		1.0f,
		1.0f,

		100.0f,
		100.0f,
		100.0f,
		100.0f,

		100.0f,
		100.0f,

		1.0f,
		1.0f,
		1.0f,

		1.0f,
		1.0f,
		1.0f,
		1.0f,
	};

	std::string IMPACT_TAR[TAR_SIZE] =
	{
		"Self",
		"Target",
		"From Self until the Target",
		"Specified location"
		"Specified"
	};

	std::string IMPACT_REALM[REALM_SIZE] =
	{
		"Target Self",
		"Target Area",
		"Continuous Area Target",
		"Cone Target"
	};

	std::string IMPACT_SIDE[SIDE_SIZE] =
	{
		"Towards my side",
		"Towards Enemy",
		"Towards Everyone"
	};

	std::string SPEC_ADDON[EMSPECA_NSIZE] =
	{
		/*
		"기능 없음",
		"당기기/밀기",
		
		"대미지 반사",
		"부활",

		"HP 빼앗음",
		"MP 빼앗음",
		"SP 빼앗음",

		"HP 나눔",
		"MP 나눔",
		"SP 나눔",

		"상태이상 방지",
		"상태이상 회복",

		"관통정도",
		"사정거리",

		"이동속도",

		"진격",
		"투명",
		"투명확인",

		"공격속도",
		"스킬 딜래이"

		"물리 데미지 흡수율",
		"마법 데미지 흡수율",
		"물리 데미지 반사율",
		"마법 데미지 반사율",
		"이로운 지속효과 제거",
		"해로운 지속효과 제거",
		"발동형 스킬"
		*/

		"None",
		"Pull/Push",
		"Reflex Damage",
		"Revive",
		"Steal Hp",
		"Steal MP",
		"Steal SP",
		"Share HP",
		"Share MP",
		"Share SP",
		"Prevent Abnormal Condition",
		"Recover Abnormal Condition",
		"Penetration",
		"Shooting Range",
		"Moving Speed",
		"Dash Attack",
		"Invisible",
		"Detect Invisible",
		"Attack Speed",
		"Skill Delay"
		"Crushing Blow Rate",
		"Melee Damage Absorption",
		"Magic Damage Absorption",
		"Reflex Melee Damage",
		"Reflex Magic Damage",
		"Remove Skill Effect",
		"Stun",
		"Active Type Skill",
	};

	std::string SPEC_ADDON_VAR1[EMSPECA_NSIZE] =
	{
		"없음",		// 기능없음.
		"거리",		// 당기기/밀기.

		"없음",		// 대미지반사.
		"회복율",	// 부활.

		"흡수율",	// HP 빼앗음.
		"흡수율",	// MP 빼앗음.
		"흡수율",	// SP 빼앗음.

		"없음",		// HP 나눔.
		"없음",		// MP 나눔.
		"없음",		// SP 나눔.

		"없음",		// 상태이상 방지.
		"없음",		// 상태이상 회복.

		"관통증가",	// 관통정도.
		"공격거리",	// 사정거리.

		"이속율",	// 이동속도.

		"거리",		// 진격
		"없음",		// 투명
		"없음",		// 투명확인

		"공속율",	// 공속율.
		"변화율",	// 스킬 딜래이 변화율
		
		"흡수율",	// 물리 데미지 흡수율
		"흡수율",	// 마법 데미지 흡수율
		"반사율",	// 물리 데미지 반사율
		"반사율",	// 마법 데미지 반사율
		"적용값",	// 이로운 지속효과 제거
		"적용값",	// 해로운 지속효과 제거
		"MID",		// 발동형 스킬
	};

	float SPEC_ADDON_VAR1_SCALE[EMSPECA_NSIZE] =
	{
		1.0f,		// 기능없음.
		1.0f,		// 당기기/밀기.

		1.0f,		// 대미지반사.
		100.0f,		// 부활.

		100.0f,		// HP 빼앗음.
		100.0f,		// MP 빼앗음.
		100.0f,		// SP 빼앗음.

		1.0f,		// HP 나눔.
		1.0f,		// MP 나눔.
		1.0f,		// SP 나눔.

		1.0f,		// 상태이상 방지.
		1.0f,		// 상태이상 회복.

		1.0f,		// 관통정도.
		1.0f,		// 사정거리.

		100.0f,		// 이동속도.

		1.0f,		// 진격
		1.0f,		// 투명
		1.0f,		// 투명확인

		100.0f,		// 공속율.
		1.0f,		// 변화율.
		
		100.0f,		// 물리 데미지 흡수율
		100.0f,		// 마법 데미지 흡수율
		100.0f,		// 물리 데미지 반사율
		100.0f,		// 마법 데미지 반사율
		1.0f,		// 이로운 지속효과 제거
		1.0f,		// 해로운 지속효과 제거
		1.0f,		// 발동형 스킬
	};

	std::string SPEC_ADDON_VAR2[EMSPECA_NSIZE] =
	{
		"없음",		// 기능없음.
		"확율",		// 당기기/밀기.

		"없음",		// 대미지반사.
		"없음",		// 부활.

		"없음",		// HP 빼앗음.
		"없음",		// MP 빼앗음.
		"없음",		// SP 빼앗음.

		"없음",		// HP 나눔.
		"없음",		// MP 나눔.
		"없음",		// SP 나눔.

		"없음",		// 상태이상 방지.
		"없음",		// 상태이상 회복.

		"없음",		// 관통정도.
		"없음",		// 사정거리.

		"없음",		// 이동속도.

		"없음",		// 진격
		"없음",		// 투명
		"없음",		// 투명확인

		"없음",		// 공속율
		"없음",		// 스킬 딜래이.

		"없음",		// 물리 데미지 흡수율
		"없음",		// 마법 데미지 흡수율
		"확률",		// 물리 데미지 반사율
		"확률",		// 마법 데미지 반사율
		"없음",		// 이로운 지속효과 제거
		"없음",		// 해로운 지속효과 제거
		"SID"		// 발동형 스킬
	};

	float SPEC_ADDON_VAR2_SCALE[EMSPECA_NSIZE] =
	{
		1.0f,		// 기능없음.
		100.0f,		// 당기기/밀기.

		1.0f,		// 대미지반사.
		1.0f,		// 부활.

		1.0f,		// HP 빼앗음.
		1.0f,		// MP 빼앗음.
		1.0f,		// SP 빼앗음.

		1.0f,		// HP 나눔.
		1.0f,		// MP 나눔.
		1.0f,		// SP 나눔.

		1.0f,		// 상태이상 방지.
		1.0f,		// 상태이상 회복.

		1.0f,		// 관통정도.
		1.0f,		// 사정거리.

		1.0f,		// 이동속도.

		1.0f,		// 진격
		1.0f,		// 투명
		1.0f,		// 투명확인

		1.0f,		// 공격속도
		1.0f,		// 스킬 딜래이

		1.0f,		// 물리 데미지 흡수율
		1.0f,		// 마법 데미지 흡수율
		100.0f,		// 물리 데미지 반사율
		100.0f,		// 마법 데미지 반사율
		1.0f,		// 이로운 지속효과 제거
		1.0f,		// 해로운 지속효과 제거
		1.0f,		// 발동형 스킬
	};

	std::string SKILLCLASS[EMSKILLCLASS_NSIZE] =
	{
		"Brawler-class1",
		"Brawler-class2",
		"Brawler-class3",
		"Brawler-class4",
		"Sword-class1",
		"Sword-class2",
		"Sword-class3",
		"Sword -class4",
		"Archery-class1",
		"Archery-class2",
		"Archery-class3",
		"Archery-class4",
		"Shaman-class1",
		"Shaman-class2",
		"Shaman-class3",
		"Shaman-class4",

		"NPC_01",
		"NPC_02",
		"NPC_03",
		"NPC_04",
		"NPC_05",
		"NPC_06",
		"NPC_07",
		"NPC_08",
		"NPC_09",
		"NPC_10",
		"NPC_11",
		"NPC_12",
		"NPC_13",
		"NPC_14",
		"Extreme-class1",
		"Extreme-class2",
		"Extreme-class3",
		"Extreme-class4",
		"NPC_19",
		"NPC_20"
	};

	std::string LANDEFFECT_TYPE[EMLANDEFFECT_SIZE] =
	{
		"Attackspeed",
		"Movespeed",
		"HPChangerate",
		"MPChangerate",
		"Recoveryrate",
		"DamageChangerate",
		"DefenseChangerate",
		"Resistance",
		"CancelAllBuff",
		"SafeZone"
	};
};