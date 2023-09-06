#include "pch.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"
#include "./GLSchoolFreePK.h"

#include "./NpcTalk.h"
#include "GLClubDeathMatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLChar::MsgSendPeriod ()
{
	m_fPERIOD_TIMER = 0.0f;

	//	Note : Period Sync
	//
	GLMSG::SNET_PERIOD NetMsgPeriod;
	NetMsgPeriod.sPRERIODTIME = GLPeriod::GetInstance().GetPeriod();

	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (LPVOID) &NetMsgPeriod );
}

void GLChar::MsgSendUpdateState ( bool bparty, bool bconfront, bool bview )
{
	//	[ÀÚ½Å¿¡°Ô] È¸º¹»óÅÂ °»½Å.
	GLMSG::SNETPC_UPDATE_STATE NetMsg;
	NetMsg.sHP = GLCHARLOGIC::m_sHP;
	NetMsg.sMP = GLCHARLOGIC::m_sMP;
	NetMsg.sSP = GLCHARLOGIC::m_sSP;

	//NetMsg.ENCODE ( m_dwGaeaID );
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	//	[ÆÄÆ¼¿øµé¿¡°Ô]
	if ( m_dwPartyID!=PARTY_NULL && bparty )
	{
		GLMSG::SNET_PARTY_MBR_POINT NetMsg;
		NetMsg.dwGaeaID = m_dwGaeaID;
		NetMsg.sHP = m_sHP;
		NetMsg.sMP = m_sMP;
		GLGaeaServer::GetInstance().SENDTOPARTYCLIENT ( m_dwPartyID, (NET_MSG_GENERIC*) &NetMsg );
	}

	//	´ë·ÃÀÚ¿¡°Ô Ã¤·Â º¯È­ ¾Ë¸².
	GLMSG::SNETPC_UPDATE_STATE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.sHP = GLCHARLOGIC::m_sHP;

	if ( m_sCONFTING.IsCONFRONTING() && bconfront )
	{
		switch ( m_sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetChar ( m_sCONFTING.dwTAR_ID );
				if ( pCHAR )
				{
					GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );
				}
			}
			break;
		case EMCONFT_PARTY:
			{
				GLPARTY_FIELD *pConftParty = GLGaeaServer::GetInstance().GetParty(m_sCONFTING.dwTAR_ID);
				if ( pConftParty )
				{
					GLPARTY_FIELD::MEMBER_ITER iter = pConftParty->m_cMEMBER.begin();
					GLPARTY_FIELD::MEMBER_ITER iter_end = pConftParty->m_cMEMBER.end();
					for ( ; iter!=iter_end; ++iter )
					{
						const GLPARTY_FNET &sMEMBER = (*iter).second;
						if ( !sMEMBER.m_bConfront )		continue;

						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetChar(sMEMBER.m_dwGaeaID);
						if ( !pTAR )					continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}
			}
			break;
		case EMCONFT_GUILD:
			{
				GLClubMan &sClubMan = GLGaeaServer::GetInstance().GetClubMan();

				GLCLUB *pMY_CLUB = sClubMan.GetClub ( m_dwGuild );
				if ( pMY_CLUB )
				{
					CLUBMEMBERS_ITER pos = pMY_CLUB->m_mapMembers.begin();
					CLUBMEMBERS_ITER end = pMY_CLUB->m_mapMembers.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetCharID((*pos).first);
						if ( !pTAR )								continue;
						if ( !pTAR->m_sCONFTING.IsCONFRONTING() )	continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}

				GLCLUB *pTAR_CLUB = sClubMan.GetClub ( m_sCONFTING.dwTAR_ID );
				if ( pTAR_CLUB )
				{
					CLUBMEMBERS_ITER pos = pTAR_CLUB->m_mapMembers.begin();
					CLUBMEMBERS_ITER end = pTAR_CLUB->m_mapMembers.end();
					for ( ; pos!=end; ++pos )
					{
						PGLCHAR pTAR = GLGaeaServer::GetInstance().GetCharID((*pos).first);
						if ( !pTAR )								continue;
						if ( !pTAR->m_sCONFTING.IsCONFRONTING() )	continue;

						GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );
					}
				}
			}
			break;
		};
	}

	//	Note : Àû´ë ÇàÀ§ÀÚÀÇ Å¸ÀÌ¸Ó °»½Å ¹× °ü¸®.
	{
		PGLCHAR pTAR = NULL;
		for ( MAPPLAYHOSTILE_ITER pos = m_mapPlayHostile.begin(); pos != m_mapPlayHostile.end(); ++pos )
		{
			pTAR = GLGaeaServer::GetInstance().GetCharID( pos->first );
			if ( pTAR )
				GLGaeaServer::GetInstance().SENDTOCLIENT ( pTAR->m_dwClientID, &NetMsgBrd );

			if( !pos->second )
				CDebugSet::ToLogFile( "GLChar::MsgSendUpdateState, pos->second = NULL" );
		}
	}

	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bClubDeathMatch = m_pLandMan->m_bClubDeathMatchMap;
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	bool bBRIGHTEVENT   = GLGaeaServer::GetInstance().IsBRIGHTEVENT();


	// ¼ºÇâÀÌº¥Æ® ÀÌ¸é ÁÖº¯¿¡µµ 
	if ( bview || bGuidBattleMap || bClubDeathMatch || bSCHOOL_FREEPK || bBRIGHTEVENT )
	{
		SendMsgViewAround((NET_MSG_GENERIC*)&NetMsgBrd);
	}
}

inline HRESULT GLChar::MsgReady ( NET_MSG_GENERIC* nmg )
{
	//	Note : È°¼ºÈ­ ½ÃÅ´.
	//
	ReSetSTATE(EM_ACT_WAITING);
	return S_OK;
}

inline HRESULT GLChar::MsgMoveState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_MOVESTATE *pNetMsg = reinterpret_cast<GLMSG::SNETPC_MOVESTATE*> ( nmg );

	DWORD dwOldActState = m_dwActState;

	if ( m_dwUserLvl >= USER_GM3 )
	{
		if ( pNetMsg->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
		else											ReSetSTATE ( EM_REQ_VISIBLENONE );
		
		if ( pNetMsg->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
		else											ReSetSTATE ( EM_REQ_VISIBLEOFF );
	}

	if ( pNetMsg->dwActState&EM_ACT_RUN )			SetSTATE(EM_ACT_RUN);
	else											ReSetSTATE(EM_ACT_RUN);

	if ( pNetMsg->dwActState & EM_ACT_PEACEMODE )	SetSTATE ( EM_ACT_PEACEMODE );
	else											ReSetSTATE ( EM_ACT_PEACEMODE );

	if ( dwOldActState != m_dwActState )
	{
		//	Note : µô·¡ÀÌ¸¦ ¹Ý¿µÇÑ ÀÌµ¿ ¼Óµµ ÃøÁ¤.
		float fVelo = GetMoveVelo ();

		m_actorMove.SetMaxSpeed ( fVelo );

		//	Note : ÀÚ½ÅÀÇ ÁÖº¯ Char¿¡°Ô ÀÚ½ÅÀÇ Msg¸¦ Àü´Þ.
		//
		GLMSG::SNETPC_MOVESTATE_BRD NetMsgFB;
		NetMsgFB.dwGaeaID = m_dwGaeaID;
		NetMsgFB.dwActState = m_dwActState;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
	}

	return S_OK;
}

// *****************************************************
// Desc: ÀÌµ¿ Ã³¸®
// *****************************************************
inline HRESULT GLChar::MsgGoto ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg ) return S_OK;
	// »óÁ¡ÀÌ °³¼³µÈ »óÅÂ¿¡¼­ ÀÌµ¿ÇÒ ¼ö ¾ø´Ù.
	if ( m_sPMarket.IsOpen() ) return E_FAIL;

	// ³»»óÅÂ°¡ DEAD ÀÌ¸é DEAD Animation Ã³¸®
	if ( IsSTATE(EM_ACT_DIE) )
	{
		//	Note : ¾²·¯ÁüÀ» Å¬¶óÀÌ¾ðÆ® µé¿¡°Ô ¾Ë¸².
		GLMSG::SNET_ACTION_BRD NetMsgBrd;
		NetMsgBrd.emCrow	= CROW_PC;
		NetMsgBrd.dwID		= m_dwGaeaID;
		NetMsgBrd.emAction	= GLAT_FALLING;

		//	Note : ÀÚ½ÅÀÇ Å¬¶óÀÌ¾ðÆ®¿¡.
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsgBrd );

		return E_FAIL;
	}

	GLMSG::SNETPC_GOTO *pNetMsg = reinterpret_cast<GLMSG::SNETPC_GOTO*> ( nmg );

	BOOL bRun = IsSTATE ( EM_ACT_RUN );				// ´Þ¸®´Â »óÅÂÀÎÁö
	BOOL bToRun = pNetMsg->dwActState&EM_ACT_RUN;   // ´Þ·Á¾ß ÇÏ´ÂÁö

	// »óÅÂ º¯°æ
	if ( bRun != bToRun )
	{
		if ( bToRun )		SetSTATE(EM_ACT_RUN);
		else				ReSetSTATE(EM_ACT_RUN);
	}

	D3DXVECTOR3 vDist = m_vPos - pNetMsg->vCurPos;
	float fDist = D3DXVec3Length(&vDist);
	if ( fDist > 60.0f )
	{
		// Á¦½ºÃÄ ÁßÀÌ¸é Á¦½ºÃÄ ³¡³»°í ( ´í½ºÁß ¼ø°£ÀÌµ¿ ¹æÁö )
		if ( m_Action == GLAT_TALK || m_Action == GLAT_GATHERING )
			return E_FAIL;
			
		TurnAction ( GLAT_IDLE );

		//	Note : ÀÚ½ÅÀÇ Å¬¶óÀÌ¾ðÆ®¿¡ À§Ä¡ °­Á¦ ¼³Á¤.
		//
		GLMSG::SNET_GM_MOVE2GATE_FB NetMsgFB;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

		//	Note : Å¬¶óÀÌ¾ðÆ®µé¿¡°Ô À§Ä¡ °­Á¦ ¼³Á¤.
		//
		GLMSG::SNETPC_JUMP_POS_BRD NetMsgJump;
		NetMsgJump.dwGaeaID = m_dwGaeaID;
		NetMsgJump.vPOS = m_vPos;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgJump );

		return S_OK;
	}

	m_TargetID.vPos = pNetMsg->vTarPos;
	TurnAction ( GLAT_MOVE );

	BOOL bSucceed = m_actorMove.GotoLocation
	(
		m_TargetID.vPos+D3DXVECTOR3(0,+10,0),
		m_TargetID.vPos+D3DXVECTOR3(0,-10,0)
	);

	if ( !bSucceed )
	{
		//CDebugSet::ToLogFile ( "m_actorMove.GotoLocation error : %f, %f, %f", m_TargetID.vPos.x, m_TargetID.vPos.y, m_TargetID.vPos.z );
	}

	if ( bSucceed )
	{
		float fVelo = GetMoveVelo ();
		m_actorMove.SetMaxSpeed ( fVelo );
	
		//	Note : ÀÚ½ÅÀÇ ÁÖº¯ Char¿¡°Ô ÀÚ½ÅÀÇ Msg¸¦ Àü´Þ.
		//
		GLMSG::SNETPC_GOTO_BRD NetMsgFB;
		NetMsgFB.dwGaeaID = m_dwGaeaID;
		NetMsgFB.dwActState = m_dwActState;
		NetMsgFB.vCurPos = m_vPos;
		NetMsgFB.vTarPos = m_TargetID.vPos;
		NetMsgFB.fDelay = 0.0f;

		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );
	}

	return S_OK;
}

inline HRESULT GLChar::MsgAttack ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg )													return S_OK;
	if ( !IsValidBody() )																			return E_FAIL;
	if ( m_pLandMan && m_pLandMan->IsPeaceZone() )													return E_FAIL;

	GLMSG::SNETPC_ATTACK *pNetMsg = reinterpret_cast<GLMSG::SNETPC_ATTACK*> ( nmg );
	m_dwANISUBSELECT = pNetMsg->dwAniSel;
	m_TargetID.emCrow = pNetMsg->emTarCrow;
	m_TargetID.dwID = pNetMsg->dwTarID;

	//	Note : Å¸°Ù °Ë»ç.
	//
	GLACTOR* pTARGET = GLGaeaServer::GetInstance().GetTarget ( m_pLandMan, m_TargetID );
	if ( !pTARGET )																					return E_FAIL;

	// Å¸°Ù°úÀÇ °Å¸® °Ë»ç
	D3DXVECTOR3 vTarPos = pTARGET->GetPosition();
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-vTarPos) );
	WORD wAttackRange = pTARGET->GetBodyRadius() + GETBODYRADIUS() + GETATTACKRANGE() + 2;
	if ( ISLONGRANGE_ARMS() )	 wAttackRange += (WORD) GETSUM_TARRANGE();
	WORD wAttackAbleDis = wAttackRange + 7;

	if ( fDist > wAttackAbleDis )
	{
		//	Note : ÀÚ½Å¿¡°Ô.
		GLMSG::SNETPC_ATTACK_AVOID NetMsg;
		NetMsg.emTarCrow	= m_TargetID.emCrow;
		NetMsg.dwTarID		= m_TargetID.dwID;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, (NET_MSG_GENERIC*) &NetMsg );

		//	Note : ÁÖº¯ Å¬¶óÀÌ¾ðÆ®µé¿¡°Ô ¸Þ¼¼Áö Àü¼Û.
		//
		GLMSG::SNETPC_ATTACK_AVOID_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID	= m_dwGaeaID;
		NetMsgBrd.emTarCrow	= m_TargetID.emCrow;
		NetMsgBrd.dwTarID	= m_TargetID.dwID;
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
			
		return E_FAIL;
	}

	bool bFreePKMap		= m_pLandMan->IsFreePK();
	bool bGuidBattleMap = m_pLandMan->m_bGuidBattleMap;
	bool bBRIGHTEVENT   = GLGaeaServer::GetInstance().IsBRIGHTEVENT();
	bool bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();
	if ( GetSchool() == pTARGET->GetSchool() )	bSCHOOL_FREEPK = false;

	bool bREACTIONALBE  = IsReActionable(pTARGET);
	if ( !bREACTIONALBE )																			return E_FAIL;

	//	Note : pc ÀÌ°í pk ¸ðµåÀÏ °æ¿ì °ø°Ý °¡´É.
	if ( pTARGET->GetCrow()==CROW_PC )
	{
		PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetChar ( m_TargetID.dwID );
		if ( pCHAR )
		{

			// »ó´ë¹æÀÌ ´ë·Ã Á¾·áÈÄ ¹«ÀûÅ¸ÀÓÀÌ¸é °ø°Ý ¹«½Ã
			if ( pCHAR->m_sCONFTING.IsPOWERFULTIME () )	
				return E_FAIL;
			// ³»°¡ ´ë·Ã Á¾·áÈÄ ¹«ÀûÅ¸ÀÓÀÌ¸é °ø°Ý ¹«½Ã
			if ( m_sCONFTING.IsPOWERFULTIME() )			
				return E_FAIL;

			bool bClubBattle = false;
			bool bClubDeathMatch = false;
			
			if ( m_pLandMan->IsClubBattleZone() )
			{	
				GLClubMan &cClubMan = m_pGLGaeaServer->GetClubMan();
				GLCLUB *pMyClub = cClubMan.GetClub ( m_dwGuild );
				GLCLUB *pTarClub = cClubMan.GetClub ( pCHAR->m_dwGuild );

				if ( pMyClub && pTarClub )
				{
					bool bClub = pMyClub->IsBattle( pCHAR->m_dwGuild );
					bool bAlliance = pMyClub->IsBattleAlliance ( pTarClub->m_dwAlliance );
					bClubBattle = (bClub || bAlliance);
				}				
			}

			if ( m_pLandMan->m_bClubDeathMatchMap )
			{
				GLClubDeathMatch* pCDM = GLClubDeathMatchFieldMan::GetInstance().Find( m_pLandMan->m_dwClubMapID );
				if ( pCDM && pCDM->IsBattle() ) bClubDeathMatch = true;
			}

			if ( !(bGuidBattleMap || bClubDeathMatch || bSCHOOL_FREEPK || bFreePKMap || bBRIGHTEVENT || IsConflictTarget(pTARGET) || bClubBattle ) )
			{
				//	Note : »ó´ë¹æÀÌ ¹üÁËÀÚ°¡ ¾Æ´Ï°í Á¤´ç¹æÀ§ ½Ã°£ÀÌ ÁöÁ¤µÇÁö ¾Ê¾ÒÀ»¶§´Â 
				//		ÀÚ½ÅÀÌ Àû´ëÇàÀ§¸¦ ½ÃÀÛÇÔ.
				if ( !IS_PLAYHOSTILE(pCHAR->m_dwCharID) && !pCHAR->ISOFFENDER() )
				{
					int nBRIGHT(GLCONST_CHAR::nPK_TRY_BRIGHT_POINT), nLIFE(GLCONST_CHAR::nPK_TRY_LIVING_POINT);

					//	Note : ¼Ó¼º¼öÄ¡ º¯È­.
					m_nBright += nBRIGHT;

					GLMSG::SNETPC_UPDATE_BRIGHT NetMsg;
					NetMsg.nBright = m_nBright;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

					GLMSG::SNETPC_UPDATE_BRIGHT_BRD NetMsgBrd;
					NetMsgBrd.dwGaeaID = m_dwGaeaID;
					NetMsgBrd.nBright = m_nBright;
					SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

					//	Note : »ýÈ°Á¡¼ö º¯È­.
					m_nLiving += nLIFE;

					GLMSG::SNETPC_UPDATE_LP NetMsgLp;
					NetMsgLp.nLP = m_nLiving;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgLp);

					//	Note : Àû´ëÇàÀ§ ·Î±× ±â·Ï.
					GLITEMLMT::GetInstance().ReqAction
					(
						m_dwCharID,					//	´ç»çÀÚ.
						EMLOGACT_HOSTILE,			//	ÇàÀ§.
						ID_CHAR, pCHAR->m_dwCharID,	//	»ó´ë¹æ.
						0,							//	exp
						nBRIGHT,					//	bright
						nLIFE,						//	life
						0							//	money
					);
				}
			}

			if ( !IsConflictTarget(pTARGET) )
			{
				//	Note : Àû´ë ÇàÀ§ÀÚ¿¡°Ô ÇÇÇØÀÚ µî·Ï. ( pk )
				AddPlayHostile ( pCHAR->m_dwCharID, TRUE, bClubBattle );

				//	Note : Àû´ë ÇÇÇØÀÚ¿¡°Ô Àû´ëÀÚ µî·Ï. ( PK )
				pCHAR->AddPlayHostile ( m_dwCharID, FALSE, bClubBattle);
			}

			STARGETID sTargetID = STARGETID ( CROW_PC, m_dwGaeaID, m_vPos );
			
			// °ø°ÝÀÚ¿Í °ø°Ý´ë»óÀÚ ¸ðµÎ ÆÖÀ» °ø°Ý¸ðµå·Î º¯°æ
			// ¸Â´ÂÀÚ
			PGLPETFIELD pEnemyPet = GLGaeaServer::GetInstance().GetPET ( pCHAR->m_dwPetGUID );
			if ( pEnemyPet && pEnemyPet->IsValid () && !pEnemyPet->IsSTATE ( EM_PETACT_ATTACK ) )
			{
				D3DXVECTOR3 vOwnerPos, vDist;
				float fDist;
				vOwnerPos = pCHAR->GetPosition ();
				vDist = pEnemyPet->m_vPos - vOwnerPos;
				fDist = D3DXVec3Length(&vDist);

				// ÀÏÁ¤°Å¸® ¾È¿¡ ÀÖÀ¸¸é 
				if ( fDist <= GLCONST_PET::fWalkArea )
				{
					pEnemyPet->ReSetAllSTATE ();
					pEnemyPet->SetSTATE ( EM_PETACT_ATTACK );

					GLMSG::SNETPET_ATTACK NetMsg;
					NetMsg.sTarID = sTargetID;
					GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsg );
					
					GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
					NetMsgBRD.dwGUID = pEnemyPet->m_dwGUID;
					NetMsgBRD.sTarID = sTargetID;
					pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
				}
			}
			// ¸Â´ÂÀÚ¸¸ Summon Attack Ã³¸®
			PGLSUMMONFIELD pEnemySummon = GLGaeaServer::GetInstance().GetSummon ( pCHAR->m_dwSummonGUID );
			if ( pEnemySummon && pEnemySummon->IsValid () && !pEnemySummon->IsSTATE ( EM_SUMMONACT_ATTACK ) && 
				pEnemySummon->GetAttackTarget( sTargetID ) )
			{
				D3DXVECTOR3 vOwnerPos, vDist;
				float fDist;
				vOwnerPos = pCHAR->GetPosition ();
				vDist = pEnemySummon->m_vPos - vOwnerPos;
				fDist = D3DXVec3Length(&vDist);

				// ÀÏÁ¤°Å¸® ¾È¿¡ ÀÖÀ¸¸é 
				//if ( fDist <= pEnemySummon->m_fWalkArea )
				{
					pEnemySummon->SetAttackTarget( sTargetID );
				}
			}

			// °ø°ÝÀÚ
			PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
			if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
			{
				D3DXVECTOR3 vDist;
				float fDist;
				vDist = pMyPet->m_vPos - m_vPos;
				fDist = D3DXVec3Length(&vDist);

				// ÀÏÁ¤°Å¸® ¾È¿¡ ÀÖÀ¸¸é 
				if ( fDist <= GLCONST_PET::fWalkArea )
				{
					pMyPet->ReSetAllSTATE ();
					pMyPet->SetSTATE ( EM_PETACT_ATTACK );

					m_TargetID.vPos = pCHAR->GetPosition ();

					GLMSG::SNETPET_ATTACK NetMsg;
					NetMsg.sTarID = m_TargetID;
					GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
					
					GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
					NetMsgBRD.dwGUID = pMyPet->m_dwGUID;
					NetMsgBRD.sTarID = m_TargetID;
					SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
				}
			}
		}
	}else if ( pTARGET->GetCrow()==CROW_SUMMON )
	{
		STARGETID sTargetID = STARGETID ( CROW_PC, m_dwGaeaID, m_vPos );

		// Summon Attack Ã³¸®
		PGLSUMMONFIELD pEnemySummon = GLGaeaServer::GetInstance().GetSummon ( m_TargetID.dwID );
		if ( pEnemySummon && pEnemySummon->IsValid () && !pEnemySummon->IsSTATE ( EM_SUMMONACT_ATTACK ) && 
			pEnemySummon->GetAttackTarget( sTargetID ) )
		{
			/*D3DXVECTOR3 vOwnerPos, vDist;
			float fDist;
			vOwnerPos = pEnemySummon->m_pOwner->GetPosition ();
			vDist = pEnemySummon->m_vPos - vOwnerPos;
			fDist = D3DXVec3Length(&vDist);

			if ( fDist <= pEnemySummon->m_fWalkArea )*/
			{
				pEnemySummon->SetAttackTarget( sTargetID );
			}
		}
	}

	EMSLOT emRHand = GetCurRHand();
	EMSLOT emLHand = GetCurLHand();
    
	EMANI_SUBTYPE emANISUBTYPE = CHECK_ATTACK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand] );

	GLCONST_CHARCLASS &ConstCharClass = GLCONST_CHAR::cCONSTCLASS[m_CHARINDEX];
	VECANIATTACK &vecAniAttack = ConstCharClass.m_ANIMATION[AN_ATTACK][emANISUBTYPE];
	if ( vecAniAttack.empty() )		return E_FAIL;
	if ( vecAniAttack.size() <= m_dwANISUBSELECT )	m_dwANISUBSELECT = 0;

	const SANIATTACK &sAniAttack = vecAniAttack[m_dwANISUBSELECT];
	WORD wStrikeNum = sAniAttack.m_wDivCount;

	m_TargetID.vPos = pTARGET->GetPosition();

	//	Note : °ø°ÝÀÌ °¡´ÉÇÑÁö °Ë»ç.
	//
	EMBEGINATTACK_FB emBeginFB = BEGIN_ATTACK(wStrikeNum);
	if ( emBeginFB!=EMBEGINA_OK && emBeginFB!=EMBEGINA_SP )		return E_FAIL;

	//	Note : ¼Ò¸ð¼º ¾ÆÀÌÅÆ À¯È¿¼º °Ë»çÈÄ, »ç¶óÁú¶§ ¸Þ½ÃÁö ¹ß»ý.
	//
	CheckInstanceItem ();

	//	Note : »çÀü ¸íÁßÄ¡ °è»ê.
	//
	BOOL bLowSP = (emBeginFB==EMBEGINA_SP) ? TRUE: FALSE;
	PreStrikeProc ( FALSE, bLowSP );

	//	Note : °ø°Ý ¼öÇà.
	//
	TurnAction ( GLAT_ATTACK );

	//	Note : °ø°Ý ¼º°ø½Ã - ÀÚ½ÅÀÇ ÁÖº¯ Char¿¡°Ô ÀÚ½ÅÀÇ Msg¸¦ Àü´Þ.
	//
	GLMSG::SNETPC_ATTACK_BRD NetMsgFB;
	NetMsgFB.dwGaeaID = m_dwGaeaID;
	NetMsgFB.emTarCrow = pNetMsg->emTarCrow;
	NetMsgFB.dwTarID = pNetMsg->dwTarID;
	NetMsgFB.dwAniSel = m_dwANISUBSELECT;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );

	// ¸÷À» °ø°ÝÇÏ¸é ÆÖÀÌ °ø°Ý¸ð¼ÇÀ» ÃëÇÔ
	if ( pTARGET->GetCrow()==CROW_MOB )
	{
		PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
		if ( pMyPet && pMyPet->IsValid () && !pMyPet->IsSTATE ( EM_PETACT_ATTACK ) )
		{
			D3DXVECTOR3 vDist;
			float fDist;
			vDist = pMyPet->m_vPos - m_vPos;
			fDist = D3DXVec3Length(&vDist);

			// ÀÏÁ¤°Å¸® ¾È¿¡ ÀÖÀ¸¸é 
			if ( fDist <= GLCONST_PET::fWalkArea )
			{
				GLMSG::SNETPET_ATTACK NetMsg;
				NetMsg.sTarID = m_TargetID;
				GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
				
				GLMSG::SNETPET_ATTACK_BRD NetMsgBRD;
				NetMsgBRD.dwGUID = m_dwPetGUID;
				NetMsgBRD.sTarID = m_TargetID;
				SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBRD );
			}
		}
	}

	return S_OK;
}

inline HRESULT GLChar::MsgAttackCancel ( NET_MSG_GENERIC* nmg )
{
	//	Note : ¾×¼Ç Ãë¼Ò.
	if ( IsACTION(GLAT_ATTACK) )	TurnAction ( GLAT_IDLE );

	//	Note : ÀÚ½ÅÀÇ ÁÖº¯ Char¿¡°Ô ÀÚ½ÅÀÇ Msg¸¦ Àü´Þ.
	//
	GLMSG::SNETPC_ATTACK_CANCEL_BRD NetMsgFB;
	NetMsgFB.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgFB );

	return S_OK;
}

inline HRESULT GLChar::MsgGatheringCancel ( NET_MSG_GENERIC* nmg )
{
	//	Note : ¾×¼Ç Ãë¼Ò.
	if ( IsACTION(GLAT_GATHERING) )	TurnAction ( GLAT_IDLE );
	return S_OK;
}

void GLChar::ReSetStartMap()
{
	// °ÔÀÌÆ®°¡ ¸ø Ã£À¸¸é ÃÖÃÊ ½ÃÀÛ À§Ä¡·Î ÀÌµ¿ÇÑ´Ù.
	m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
	m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
	m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);

	// ½ÃÀÛÀ§Ä¡ ¸®¼Â (°¢ÇÐ¿øº° ½ÃÀÛ¸ÊÀ¸·Î)
	GLMSG::SNETPC_UPDATE_STARTCALL NetMsg;
	NetMsg.sStartMapID   = m_sStartMapID;
	NetMsg.dwStartGateID = m_dwStartGate;
	NetMsg.vStartPos	 = m_vStartPos;

	// Å¬¶óÀÌ¾ðÆ®¿¡ ¾Ë¸²
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	// Agent¿¡ ¾Ë¸²
	GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsg );		

	return;
}

BOOL GLChar::CheckStartMap()
{
	GLLandMan *pLandMan=NULL;
	DWORD dwGenGate = 0;
	pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );

	if ( pLandMan && pLandMan->m_bClubDeathMatchMap )
	{
		if ( GLCONST_CHAR::bPKLESS ) return FALSE;	

	    GLClubDeathMatch *pCDM = GLClubDeathMatchFieldMan::GetInstance().Find ( pLandMan->m_dwClubMapID );
		if ( pCDM && !pCDM->IsEnterMap( m_dwGuild ) )	return FALSE;
		
		GLCLUB* pClub = GLGaeaServer::GetInstance().GetClubMan().GetClub ( m_dwGuild );
		if ( !pClub ) return FALSE;
		if ( pClub->GetAllBattleNum() > 0 ) return FALSE;
		if ( !pClub->IsMemberFlgCDM(m_dwCharID) )	return FALSE;
	}

	if ( pLandMan && pLandMan->GetLandGateMan().FindLandGate(m_dwStartGate) ) 		
	{
		return TRUE;
	}
	
	return FALSE;

}


inline void GLChar::DoReBirth2StartMap ()
{
	SNATIVEID nidSTARTMAP = m_sStartMapID;
	DWORD dwSTARTGATE = m_dwStartGate;

	//	ºÎÈ°.
	GLLandMan *pLandMan=NULL;
	DWORD dwGenGate = 0;


	//	Á¤»óÀûÀÎ »óÈ² 
	if ( CheckStartMap() )
	{
		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );
		if ( pLandMan )
		{
			BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), dwSTARTGATE, D3DXVECTOR3(0,0,0) );
			if ( !bOk )	DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
			return;
		}
		else
		{
			//	³ª¿Ã¼ö ¾ø´Â »óÈ²
			DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () pLandMan NULL" );
			return;
		}
	}
	else	//	½ÃÀÛÀ§Ä¡·Î ÀÌµ¿
	{
		// °ÔÀÌÆ®°¡ ¸ø Ã£À¸¸é ÃÖÃÊ ½ÃÀÛ À§Ä¡·Î ÀÌµ¿ÇÑ´Ù.
		ReSetStartMap();

		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );
		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( !pLandGateMan->FindLandGate ( m_dwStartGate ) )
			{
				DEBUGMSG_WRITE ( "(Revive) - Default gate is a failure. school %d", m_wSchool );
				return;
			}
			else
			{
//				m_bEntryFailed = TRUE;
				BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), m_dwStartGate, D3DXVECTOR3(0,0,0) );	
				if ( !bOk )	DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
				return;
			}
		}
		else
		{
			DEBUGMSG_WRITE ( "(Revive) - Default map is a failure. school %d", m_wSchool );
			return;
		}
	}

	return;

/*


	//	ºÎÈ°ÇÒ Map°ú Gate À¯È¿¼º °Ë»ç.
	if ( nidSTARTMAP!=NATIVEID_NULL() )
	{
		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );

		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( pLandGateMan->FindLandGate(dwSTARTGATE)==NULL )
			{
				//PDXLANDGATE pLandGate = pLandGateMan->GetListLandGate();
				//if ( !pLandGate )	pLandMan = NULL;	//	Gate°¡ ÇÏ³ªµµ ¾ø´Â °÷¿£ ºÎÈ°ÇÒ ¼ö ¾øÀ½.
				//else
				//{
				//	dwGenGate = pLandGate->GetGateID();	//	Ã¹¹øÂ° Gate ¿¡¼­ »ý±è.
				//}
				// °ÔÀÌÆ®°¡ ¸ø Ã£À¸¸é ÃÖÃÊ ½ÃÀÛ À§Ä¡·Î ÀÌµ¿ÇÑ´Ù.
				m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
				m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
				m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);

				// ½ÃÀÛÀ§Ä¡ ¸®¼Â (°¢ÇÐ¿øº° ½ÃÀÛ¸ÊÀ¸·Î)
				GLMSG::SNETPC_UPDATE_STARTCALL NetMsg;
				NetMsg.sStartMapID   = m_sStartMapID;
				NetMsg.dwStartGateID = m_dwStartGate;
				NetMsg.vStartPos	 = m_vStartPos;

				// Å¬¶óÀÌ¾ðÆ®¿¡ ¾Ë¸²
				GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

				// Agent¿¡ ¾Ë¸²
				GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsg );

				pLandMan = GLGaeaServer::GetInstance().GetByMapID ( m_sStartMapID );
				DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
				if ( !pLandGateMan->FindLandGate ( m_dwStartGate ) )
				{
					DEBUGMSG_WRITE ( "Gate == NULL (Revive) - Default gate is a failure. school %d", m_wSchool );
					return;
				}else{
					dwGenGate = m_dwStartGate;
				}

				m_bEntryFailed = TRUE;
			}
			else	dwGenGate = dwSTARTGATE;
		}
	}

	if ( !pLandMan )
	{
		SNATIVEID nidSTARTMAP = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
		DWORD dwSTARTGATE = GLCONST_CHAR::dwSTARTGATE[m_wSchool];

		pLandMan = GLGaeaServer::GetInstance().GetByMapID ( nidSTARTMAP );
		if ( pLandMan )
		{
			DxLandGateMan* pLandGateMan = &pLandMan->GetLandGateMan();
			if ( !pLandGateMan->FindLandGate ( dwSTARTGATE ) )
			{
				DEBUGMSG_WRITE ( "(Revive) - Default gate is a failure. school %d", m_wSchool );
				return;
			}
			else
			{
				dwGenGate = dwSTARTGATE;
			}
		}
	}

	if ( !pLandMan )
	{
		DEBUGMSG_WRITE ( "(Revive) - Default map is a failure. school %d", m_wSchool );
		return;
	}

	//	Note : ºÎÈ° map°ú ºÎÈ° gate ÁöÁ¤, À§Ä¡´Â ÀÇ¹Ì ¾ø´Â °ª.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, pLandMan->GetMapID(), dwGenGate, D3DXVECTOR3(0,0,0) );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "GLGaeaServer::GetInstance().RequestReBirth () bOk false" );
		return;
	}
*/
}

// *****************************************************
// Desc: ºÎÈ°¿äÃ» Ã³¸® ( Ageng->Field )
// *****************************************************
inline HRESULT GLChar::MsgReqReBirth ( NET_MSG_GENERIC* nmg )
{
	if ( !IsSTATE(EM_ACT_DIE) )
	{
		DEBUGMSG_WRITE ( "GLChar::MsgReqReBirth IsSTATE(EM_ACT_DIE) false" );
		return S_OK;
	}

	//	ºÎÈ° ¿Ï·á Ã¼Å©.
	ReSetSTATE(EM_ACT_DIE);

	//  ºÎÈ° ÁöÁ¡À¸·Î ÁøÀÔ ºÒ°¡½Ã ºÎÈ° ÁöÁ¡À» ÃÊ±âÈ­ÇÑ´Ù.
	GLMSG::SNETPC_REQ_REBIRTH *pNetMsg = (GLMSG::SNETPC_REQ_REBIRTH *) nmg;
	if( pNetMsg->bRegenEntryFailed )
	{
		m_sStartMapID = GLCONST_CHAR::nidSTARTMAP[m_wSchool];
		m_dwStartGate = GLCONST_CHAR::dwSTARTGATE[m_wSchool];
		m_vStartPos   = D3DXVECTOR3(0.0f,0.0f,0.0f);
	}

	//	Note : ÁöÁ¤µÈ ºÎÈ°Áö¿ª¿¡¼­ ºÎÈ°µÇµµ·Ï À§Ä¡ ÀÌµ¿.
	//
	DoReBirth2StartMap ();

	//	ºÎÈ°½Ã Ã¤·Â È¸º¹.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER();
	TurnAction ( GLAT_IDLE );

	//	Note : ºÎÈ°½Ã °æÇèÄ¡ °¨¼Ò.
	ReBirthDecExp ();

	// PET
	// ¸ÊÀÌµ¿½Ã Pet »èÁ¦
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	//	ºÎÈ° È®ÀÎ ¸Þ½ÃÁö.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.wNowHP = m_sHP.wNow;
	NetMsg.wNowMP = m_sMP.wNow;
	NetMsg.wNowSP = m_sSP.wNow;
	NetMsg.bRegenEntryFailed = pNetMsg->bRegenEntryFailed;

	//	Note : Á×À»¶§ ·Î±× ±â·Ï.
	// ºÎÈ°ÇÏ±â ¹öÆ°ÀÌ ³ª¿ÀÁö ¾Ê¾Æ ·Î±×¸¦ ¸ø³²±â´Â °æ¿ì°¡ ÀÖ¾î¼­ Á×À¸¸é ¹«Á¶°Ç ³²±ä´Ù.
	//{
	//	EMIDTYPE emKILL = ID_MOB;
	//	if ( m_sAssault.emCrow==CROW_PC )	emKILL = ID_CHAR;

	//	GLITEMLMT::GetInstance().ReqAction
	//		(
	//		m_dwCharID,					//	´ç»çÀÚ.
	//		EMLOGACT_DIE,				//	ÇàÀ§.
	//		emKILL, 0,					//	»ó´ë¹æ.
	//		m_nDECEXP,					// exp
	//		0,							// bright
	//		0,							// life
	//		0							// money
	//		);
	//}


	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	return S_OK;
}

// *****************************************************
// Desc: ºÎÈ°¿äÃ» Ã³¸® (±ÍÈ¥ÁÖ »ç¿ë)
// *****************************************************
inline HRESULT GLChar::MsgReqRevive ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_REVIVE *pNetMsg = (GLMSG::SNETPC_REQ_REVIVE *)nmg;
	GLMSG::SNETPC_REQ_REVIVE_FB MsgFB;

	// ¾Æ¾ÆÅÛÀÌ ÀÖ´ÂÁö
	SITEM* pITEM = GET_SLOT_ITEMDATA(SLOT_NECK);
	if ( !pITEM )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( CheckCoolTime ( pITEM->sBasicOp.sNativeID ) )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_COOLTIME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

/*
	// ±ÍÈ¥ÁÖ ¿©ºÎ
	if ( pITEM->sBasicOp.emItemType != ITEM_REVIVE )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}
*/
	// ¾ÆÀÌÅÛ ¼Ó¼ºÀÌ ºÎÈ°±â´ÉÀÎÁö
	if ( pITEM->sDrugOp.emDrug!=ITEM_DRUG_CALL_REVIVE )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// ¼±µµ¸ÊÀÎÁö
	if ( m_pLandMan && m_pLandMan->m_bGuidBattleMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// Å¬·´µ¥½º¸ÅÄ¡ Á¸
	if ( m_pLandMan->m_bClubDeathMatchMap )
	{
		MsgFB.emFB = EMREQ_REVIVE_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	//	Note : ±ÍÈ¥ÁÖ »ç¿ë.
	//SITEMCUSTOM &sCUSTOM = m_PutOnItems[SLOT_NECK];

	//	Note : ¼Ò¸ðµÈ ±ÍÈ¥ÁÖ Ã³¸®.
	//
	DoDrugSlotItem(SLOT_NECK);

	//	Note : ºÎÈ° map°ú ºÎÈ° gate ÁöÁ¤, À§Ä¡´Â ÀÇ¹Ì ¾ø´Â °ª.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), UINT_MAX, m_vPos );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "Ä³¸¯ÅÍ ºÎÈ° ½ÃµµÁß¿¡ ¿À·ù°¡ ¹ß»ý! GLGaeaServer::GetInstance().RequestReBirth ()" );
		return E_FAIL;
	}

	//	ºÎÈ° ¿Ï·á Ã¼Å©.
	ReSetSTATE(EM_ACT_DIE);

	//	ºÎÈ°½Ã Ã¤·Â È¸º¹.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );	

	// PET
	// ¸ÊÀÌµ¿½Ã Pet »èÁ¦
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	//	ºÎÈ° È®ÀÎ ¸Þ½ÃÁö.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.wNowHP = m_sHP.wNow;
	NetMsg.wNowMP = m_sMP.wNow;
	NetMsg.wNowSP = m_sSP.wNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	// ±ÍÈ¥ÁÖ »ç¿ë ºÎÈ° ¼º°øÀ» ¾Ë¸²
	MsgFB.emFB = EMREQ_REVIVE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	////	Note : Á×À»¶§ ·Î±× ±â·Ï.
	// ºÎÈ°ÇÏ±â ¹öÆ°ÀÌ ³ª¿ÀÁö ¾Ê¾Æ ·Î±×¸¦ ¸ø³²±â´Â °æ¿ì°¡ ÀÖ¾î¼­ ±ÍÈ¥ÁÖ »ç¿ë½Ã ·Î±×¸¦ ³²±ä´Ù.
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	´ç»çÀÚ.
			EMLOGACT_RESTORE,			//	ÇàÀ§.
			ID_CHAR, 0,					//	»ó´ë¹æ.
			m_nDECEXP,					// exp
			0,							// bright
			0,							// life
			0							// money
			);
	}

	return S_OK;
}

HRESULT GLChar::MsgReqRecovery ( NET_MSG_GENERIC* nmg )
{
	// °æÇèÄ¡È¸º¹_Á¤ÀÇ_Normal
	GLMSG::SNETPC_REQ_RECOVERY *pNetMsg = (GLMSG::SNETPC_REQ_RECOVERY *)nmg;
	GLMSG::SNETPC_REQ_RECOVERY_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = GetReExp();
	nDecMoney = GetReExpMoney ( nReExp );

	if ( nDecMoney > m_lnMoney )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOMONEY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( nReExp <= 0  )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOREEXP;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// ¼±µµ¸ÊÀÎÁö
	if ( m_pLandMan && m_pLandMan->m_bGuidBattleMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// Å¬·´µ¥½º¸ÅÄ¡Á¸
	if ( m_pLandMan->m_bClubDeathMatchMap )
	{
		MsgFB.emFB = EMREQ_RECOVERY_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	//	Note : ºÎÈ° map°ú ºÎÈ° gate ÁöÁ¤, À§Ä¡´Â ÀÇ¹Ì ¾ø´Â °ª.
	BOOL bOk = GLGaeaServer::GetInstance().RequestReBirth ( m_dwGaeaID, m_pLandMan->GetMapID(), UINT_MAX, m_vPos );
	if ( !bOk )
	{
		DEBUGMSG_WRITE ( "Ä³¸¯ÅÍ ºÎÈ° ½ÃµµÁß¿¡ ¿À·ù°¡ ¹ß»ý! GLGaeaServer::GetInstance().RequestReBirth ()" );
		return E_FAIL;
	}

	//	ºÎÈ° ¿Ï·á Ã¼Å©.
	ReSetSTATE(EM_ACT_DIE);

	//	ºÎÈ°½Ã Ã¤·Â È¸º¹.
	m_fGenAge = 0.0f;
	GLCHARLOGIC::INIT_RECOVER(100);
	TurnAction ( GLAT_IDLE );
	
	if ( m_nDECEXP > m_sExperience.lnNow ) m_nDECEXP = m_sExperience.lnNow;

	if ( m_nDECEXP >= nReExp ) m_nDECEXP -= nReExp;

	CheckMoneyUpdate( m_lnMoney, nDecMoney, FALSE, "Recovery Exp" );
	m_bMoneyUpdate = TRUE;

	m_lnMoney -= nDecMoney;

	ReBirthDecExp ();

	m_lnReExp = 0;	// °æÇèÄ¡ È¸º¹ °¡´É·® 0À¸·Î ÃÊ±âÈ­

	// PET
	// ¸ÊÀÌµ¿½Ã Pet »èÁ¦
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	//	ºÎÈ° È®ÀÎ ¸Þ½ÃÁö.
	GLMSG::SNETPC_REQ_REBIRTH_FB NetMsg;
	NetMsg.sMapID = m_pLandMan->GetMapID();
	NetMsg.vPos = GetPosition();
	NetMsg.wNowHP = m_sHP.wNow;
	NetMsg.wNowMP = m_sMP.wNow;
	NetMsg.wNowSP = m_sSP.wNow;

	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);

	// °æÇèÄ¡ È¸º¹ ¼º°ø ¾Ë¸²
	MsgFB.emFB = EMREQ_RECOVERY_FB_OK;
	MsgFB.nReExp = nReExp;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

	
	//	°æÇèÄ¡ ºÎÈ° ·Î±×
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	´ç»çÀÚ.
			EMLOGACT_RECOVERY,			//	ÇàÀ§.
			ID_CHAR, 0,					//	»ó´ë¹æ.
			nReExp,						// exp
			0,							// bright
			0,							// life
			(int)nDecMoney				// money
			);
	}

	return S_OK;
}


// º¹±¸ÇÒ °æÇèÄ¡·®À» ¹¯´Â´Ù.
HRESULT GLChar::MsgReqGetExpRecovery ( NET_MSG_GENERIC* nmg )
{
	// °æÇèÄ¡È¸º¹_Á¤ÀÇ_Normal
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY *pNetMsg = (GLMSG::SNETPC_REQ_GETEXP_RECOVERY *)nmg;
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;
	LONGLONG nDecExp = 0;

	nDecExp = m_nDECEXP;
	if ( nDecExp > m_sExperience.lnNow ) nDecExp = m_sExperience.lnNow;

	nReExp = GetReExp();
	nDecMoney = GetReExpMoney ( nReExp );

	
	MsgFB.nDecExp = nDecExp;
	MsgFB.nReExp = nReExp;
	MsgFB.nDecMoney = nDecMoney;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

//	È¸º¹ÇÒ °æÇèÄ¡·®À» ¾Ë·ÁÁØ´Ù. NPC
HRESULT GLChar::MsgReqGetExpRecoveryNpc ( NET_MSG_GENERIC* nmg )
{
// °æÇèÄ¡È¸º¹_Á¤ÀÇ_Npc
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC *pNetMsg = (GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC *)nmg;
	GLMSG::SNETPC_REQ_GETEXP_RECOVERY_NPC_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = m_lnReExp;
	nDecMoney = GetReExpMoney ( nReExp );	

	MsgFB.nReExp = nReExp;
	MsgFB.nDecMoney = nDecMoney;
	MsgFB.dwNPCID = pNetMsg->dwNPCID;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

//	°æÇèÄ¡¸¦ È¸º¹ÇÑ´Ù.
HRESULT GLChar::MsgReqRecoveryNpc ( NET_MSG_GENERIC* nmg )
{
	// °æÇèÄ¡È¸º¹_Á¤ÀÇ_Npc
	GLMSG::SNETPC_REQ_RECOVERY_NPC *pNetMsg = (GLMSG::SNETPC_REQ_RECOVERY_NPC *)nmg;
	GLMSG::SNETPC_REQ_RECOVERY_NPC_FB MsgFB;

	LONGLONG nReExp = 0;
	LONGLONG nDecMoney = 0;

	nReExp = m_lnReExp;
	nDecMoney = GetReExpMoney ( nReExp );

	if ( nDecMoney > m_lnMoney )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOMONEY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	if ( nReExp <= 0  )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOREEXP;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	// NPC Ã¼Å© 
	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNPCID );
	if ( !pCrow )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pCrow->GetPosition()) );
	float fTalkRange = (float) (pCrow->GetBodyRadius() + GETBODYRADIUS() + 30);
	float fTalkableDis = fTalkRange + 20;

	if ( fDist>fTalkableDis )
	{
		MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_NOTUSE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return E_FAIL;
	}

	CheckMoneyUpdate( m_lnMoney, nDecMoney, FALSE, "Recovery Exp To Npc" );
	m_bMoneyUpdate = TRUE;

	m_sExperience.lnNow += m_lnReExp;
	m_lnMoney -= nDecMoney;
	m_lnReExp = 0;	// °æÇèÄ¡ È¸º¹ °¡´É·® 0À¸·Î ÃÊ±âÈ­		
	
	// °æÇèÄ¡ È¸º¹ ¼º°ø ¾Ë¸²
	MsgFB.emFB = EMREQ_RECOVERY_NPC_FB_OK;
	MsgFB.nReExp = nReExp;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

	
	//	°æÇèÄ¡ ºÎÈ° ·Î±×
	{

		GLITEMLMT::GetInstance().ReqAction
			(
			m_dwCharID,					//	´ç»çÀÚ.
			EMLOGACT_RECOVERY_NPC,		//	ÇàÀ§.
			ID_CHAR, 0,					//	»ó´ë¹æ.
			nReExp,						// exp
			0,							// bright
			0,							// life
			(int)nDecMoney				// money
			);
	}
	
	return S_OK;


}

// *****************************************************
// Desc: Ä£±¸ÀÌµ¿ Ã³¸® (Agent->Field)
// *****************************************************
inline HRESULT GLChar::MsgReqFriendFld ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_2_FRIEND_FLD *pNetMsg = (GLMSG::SNETPC_2_FRIEND_FLD *)nmg;
	GLMSG::SNETPC_2_FRIEND_FB MsgFB;

	//	Note : ¾ÆÀÌÅÛ Á¸Àç À¯¹« ÆÇ´Ü.
	SINVENITEM *pINVENITEM = m_cInventory.GetItem ( pNetMsg->wItemPosX, pNetMsg->wItemPosY );
	if ( !pINVENITEM )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	if ( CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return S_FALSE;

	SITEM *pITEM_DATA = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM_DATA )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	if ( pITEM_DATA->sBasicOp.emItemType!=ITEM_2FRIEND )
	{
		MsgFB.emFB = EM2FRIEND_FB_NO_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return S_FALSE;
	}

	//	ÄÉ¸¯ÀÌ Á¤»ó »óÅÂ°¡ ¾Æ´Ò °æ¿ì.
	if ( !IsValidBody() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	´ë·Ã µµÁßÀÏ °æ¿ì.
	if ( m_sCONFTING.IsCONFRONTING() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	°Å·¡ÁßÀÏ °æ¿ì.
	if ( m_sTrade.Valid() )
	{
		MsgFB.emFB = EM2FRIEND_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SMAPNODE *pMapNode = GLGaeaServer::GetInstance().FindMapNode ( pNetMsg->sFriendMapID );
	if ( !pMapNode )
	{
		MsgFB.emFB = EM2FRIEND_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( pMapNode->bBattleZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( pMapNode->bCDMZone )
	{
		MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	Note : ÇØ´ç ·¹º§ÀÇ ÁøÀÔ Á¶°Ç °Ë»ç.
	const SLEVEL_REQUIRE* pLEVEL_REQUIRE = GLGaeaServer::GetInstance().GetLevelRequire(pNetMsg->sFriendMapID);
	if ( !pLEVEL_REQUIRE )
	{
		MsgFB.emFB = EM2FRIEND_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	if ( m_dwUserLvl < USER_GM3 )
	{
		if ( pLEVEL_REQUIRE->ISCOMPLETE(this)!=EMREQUIRE_COMPLETE )
		{
			MsgFB.emFB = EM2FRIEND_FB_MAP_CONDITION;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
			return FALSE;
		}
	}

	//	Note : pk µî±ÞÀÌ »ìÀÎÀÚ µî±Þ ÀÌ»óÀÏ °æ¿ì ±ÍÈ¯ Ä«µåÀÇ »ç¿ëÀ» ¸·´Â´Ù.
	//
	DWORD dwPK_LEVEL = GET_PK_LEVEL();
	if ( dwPK_LEVEL != UINT_MAX && dwPK_LEVEL>GLCONST_CHAR::dwPK_RECALL_ENABLE_LEVEL )
	{
		MsgFB.emFB = EM2FRIEND_FB_PK_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return E_FAIL;
	}

	//	Note : ¾ÆÀÌÅÛ ¼Ò¸ð.
	DoDrugInvenItem ( pNetMsg->wItemPosX, pNetMsg->wItemPosY, true );

	SNATIVEID sMAPID = pNetMsg->sFriendMapID;
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS = pNetMsg->vFriendPos;

	// PET
	// ¸ÊÀÌµ¿½Ã Pet »èÁ¦
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,true,true) );
	GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SaveVehicle( m_dwClientID, m_dwGaeaID, true );

	//	Note : ´Ù¸¥ ÇÊµå ¼­¹öÀÏ °æ¿ì.
	if ( pMapNode->dwFieldSID!=GLGaeaServer::GetInstance().GetFieldSvrID() )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAg );
	}
	//	Note : °°Àº ÇÊµå ¼­¹öÀÏ °æ¿ì.
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = m_sMapID;

		BOOL bOK = GLGaeaServer::GetInstance().RequestInvenRecallThisSvr ( this, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ¹öÇÁ¿Í »óÅÂÀÌ»ó Á¦°Å
		if ( sCurMapID != sMAPID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		DISABLESKEFF ( i );
		}

		ResetAction();

		//	Note : ¸ã ÀÌµ¿ ¼º°øÀ» ¾Ë¸².
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

inline HRESULT GLChar::MsgReqMove2CharFld ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_GM_MOVE2CHAR_FLD *pNetMsg = (GLMSG::SNETPC_GM_MOVE2CHAR_FLD *)nmg;
	GLMSG::SNETPC_GM_MOVE2CHAR_FB MsgFB;

	//	ÄÉ¸¯ÀÌ Á¤»ó »óÅÂ°¡ ¾Æ´Ò °æ¿ì.
	if ( !IsValidBody() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	´ë·Ã µµÁßÀÏ °æ¿ì.
	if ( m_sCONFTING.IsCONFRONTING() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	//	°Å·¡ÁßÀÏ °æ¿ì.
	if ( m_sTrade.Valid() )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_MY_CONDITION;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SMAPNODE *pMapNode = GLGaeaServer::GetInstance().FindMapNode ( pNetMsg->sToMapID );
	if ( !pMapNode )
	{
		MsgFB.emFB = EMGM_MOVE2CHAR_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &MsgFB );
		return FALSE;
	}

	SNATIVEID sMAPID = pNetMsg->sToMapID;
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS = pNetMsg->vToPos;

	// PET
	// ¸ÊÀÌµ¿½Ã Pet »èÁ¦
	GLGaeaServer::GetInstance().ReserveDropOutPet ( SDROPOUTPETINFO(m_dwPetGUID,false,true) );
	GLGaeaServer::GetInstance().ReserveDropOutSummon ( SDROPOUTSUMMONINFO(m_dwSummonGUID,true) );
	GLGaeaServer::GetInstance().SetActiveVehicle( m_dwClientID, m_dwGaeaID, false );

	//	Note : ´Ù¸¥ ÇÊµå ¼­¹öÀÏ °æ¿ì.
	if ( pMapNode->dwFieldSID!=GLGaeaServer::GetInstance().GetFieldSvrID() )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAg );
	}
	//	Note : °°Àº ÇÊµå ¼­¹öÀÏ °æ¿ì.
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = m_sMapID;

		BOOL bOK = GLGaeaServer::GetInstance().RequestInvenRecallThisSvr ( this, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ¹öÇÁ¿Í »óÅÂÀÌ»ó Á¦°Å
		if ( sCurMapID != sMAPID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		DISABLESKEFF ( i );
		}

		ResetAction();

		//	Note : ¸ã ÀÌµ¿ ¼º°øÀ» ¾Ë¸².
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

inline HRESULT GLChar::MsgReqLevelUp ( NET_MSG_GENERIC* nmg )
{
	if( GLGaeaServer::GetInstance().m_bEmptyMsg ) return S_OK;

	if ( VALID_LEVELUP() )
	{

		//	·¡¹ë¾÷ Ã³¸®.
		LEVLEUP ( !m_bVietnamLevelUp );

		//	Note : ·¾¾÷¿¡ µû¸¥ Äù½ºÆ® ÀÚµ¿ ½ÃÀÛ.
		//
		QuestStartFromGetLEVEL ( m_wLevel );

		//	Note : Äù½ºÆ® ÁøÇà Á¶°Ç µµ´Þ ¿©ºÎ.
		//
		DoQuestLevel ();

		//	[ÀÚ½Å¿¡°Ô]
		GLMSG::SNETPC_REQ_LEVELUP_FB NetMsg;
		NetMsg.wLevel		 = m_wLevel;
		NetMsg.wStatsPoint	 = m_wStatsPoint;
		NetMsg.dwSkillPoint  = m_dwSkillPoint;
		NetMsg.bInitNowLevel = !m_bVietnamLevelUp;
		NetMsg.sMapID		 = m_sMapID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

		//	[¸ðµÎ¿¡°Ô]
		GLMSG::SNETPC_REQ_LEVELUP_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );
	
		//	[ÆÄÆ¼,´ë·ÃÀÚ] hp º¯È­ ¾Ë¸².
		MsgSendUpdateState();


		//	Note : ·¹º§¾÷ ·Î±× ±â·Ï.(·¹º§ 20ÀÌ»ó¿¡¼­)
		if( m_wLevel > 20 )
		{
			GLITEMLMT::GetInstance().ReqAction( m_dwCharID,				//	´ç»çÀÚ.
												EMLOGACT_LEV_UP,		//	ÇàÀ§.
												ID_CHAR, 0,				//	»ó´ë¹æ.
												m_sExperience.lnNow,	//	exp
												0,						//	bright
												0,						//	life
												0 );					//	money
		}

#if defined(_RELEASED) // ***Tracing Log print
		if( m_bTracingUser )
		{
			NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
			TracingMsg.nUserNum  = GetUserID();
			StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, m_szUID );

			CString strTemp;
			strTemp.Format( "LevelUp!!, [%s][%s], Level[%d]", m_szUID, m_szName, m_wLevel );

			StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

			m_pGLGaeaServer->SENDTOAGENT( m_dwClientID, &TracingMsg );
		}
#endif

		//** Add EventTime

		SEventState sEventState = GLGaeaServer::GetInstance().m_sEventState;
		if( sEventState.bEventStart )
		{
			if( m_bEventApply == TRUE )
			{
				if( sEventState.MinEventLevel > GETLEVEL() || sEventState.MaxEventLevel < GETLEVEL() )
				{
					m_bEventApply = FALSE;
				}
			}else{
				if( sEventState.MinEventLevel <= GETLEVEL() && sEventState.MaxEventLevel >= GETLEVEL() )
				{
					CTime	  crtTime     = CTime::GetCurrentTime();
					m_sEventTime.loginTime = crtTime.GetTime();

					GLMSG::SNET_GM_LIMIT_EVENT_RESTART NetMsg;
					NetMsg.restartTime = crtTime.GetTime();
					GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&NetMsg);
					m_bEventApply = TRUE;
				}
			}
		}

		if( m_bVietnamLevelUp )
		{
			if ( m_sExperience.ISOVER() )
			{
				//	Note : ·¦¾÷ ¼öÇà. ( ·ÎÄÃ ¸Þ½ÃÁö ¹ß»ý. )
				//
				GLMSG::SNETPC_REQ_LEVELUP NetMsg;
				MsgReqLevelUp ( (NET_MSG_GENERIC*) &NetMsg );

				m_lnLastSendExp = m_sExperience.lnNow;
			}
		}

		
	}else{
		if( m_CHARINDEX == GLCI_EXTREME_M || m_CHARINDEX == GLCI_EXTREME_W )
		{
			if ( m_wLevel > GLCONST_CHAR::wMAX_EXTREME_LEVEL )
			{
				HACKINGLOG_WRITE( "Level up Failed!!, Account[%s], ID[%s], Level %d, NowExp %d, MaxExp %d", 
					m_szUID, m_szName, m_wLevel, m_sExperience.lnNow, m_sExperience.lnMax );	

				m_sExperience.lnMax = GLOGICEX::GLNEEDEXP2(GLCONST_CHAR::wMAX_EXTREME_LEVEL);
			}
		}else{
			if ( m_wLevel > GLCONST_CHAR::wMAX_LEVEL )	
			{
				HACKINGLOG_WRITE( "Level up Failed!!, Account[%s], ID[%s], Level %d, NowExp %d, MaxExp %d", 
					m_szUID, m_szName, m_wLevel, m_sExperience.lnNow, m_sExperience.lnMax );

				m_sExperience.lnMax = GLOGICEX::GLNEEDEXP(GLCONST_CHAR::wMAX_LEVEL);
			}
		}
		return E_FAIL;
	}

	m_bVietnamLevelUp = FALSE;

	return S_OK;
}

inline HRESULT GLChar::MsgReqStatsUp ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_STATSUP *pNetMsg = (GLMSG::SNETPC_REQ_STATSUP *) nmg;
	if ( m_wStatsPoint == 0 )	return S_FALSE;

	//	¼±ÅÃÇÑ STATE¸¦ Áõ°¡½ÃÅ´.
	STATSUP(pNetMsg->emStats);

	//	Ã³¸® °á°ú ÀÀ´ä.
	GLMSG::SNETPC_REQ_STATSUP_FB NetMsg;
	NetMsg.emStats = pNetMsg->emStats;
	NetMsg.wStatsPoint = m_wStatsPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg);

	//	[ÆÄÆ¼,´ë·ÃÀÚ] hp º¯È­ ¾Ë¸².
	MsgSendUpdateState();

	return S_OK;
}

//add statsup cmd
inline HRESULT GLChar::MsgReqStatsUpCmd(NET_MSG_GENERIC* nmg)
{
	GLMSG::SNETPC_REQ_STATSUPCMD* pNetMsg = (GLMSG::SNETPC_REQ_STATSUPCMD*)nmg;
	if (m_wStatsPoint < pNetMsg->dwValue)	return S_FALSE;

	//	������ STATE�� ������Ŵ.
	STATSUP_CMD(pNetMsg->emStats, pNetMsg->dwValue);

	//	ó�� ��� ����.
	GLMSG::SNETPC_REQ_STATSUPCMD_FB NetMsg;
	NetMsg.emStats = pNetMsg->emStats;
	NetMsg.dwValue = pNetMsg->dwValue;
	NetMsg.wStatsPoint = m_wStatsPoint;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &NetMsg);

	//	[��Ƽ,�����] hp ��ȭ �˸�.
	MsgSendUpdateState();

	return S_OK;
}

inline HRESULT GLChar::MsgReqActionQSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ACTIONQUICK_SET *pNetMsg = (GLMSG::SNETPC_REQ_ACTIONQUICK_SET *) nmg;

	WORD wSLOT = pNetMsg->wSLOT;
	if ( EMACTIONQUICK_SIZE <= wSLOT )		return E_FAIL;
	if ( !VALID_HOLD_ITEM() )				return S_FALSE;

	const SITEMCUSTOM& sITEM = GET_HOLD_ITEM ();

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sITEM.sNativeID );
	if ( !pITEM )							return S_FALSE;

	if ( pITEM->sBasicOp.emItemType != ITEM_CURE && 
		 pITEM->sBasicOp.emItemType != ITEM_RECALL &&
		 pITEM->sBasicOp.emItemType != ITEM_TELEPORT_CARD &&
		 pITEM->sBasicOp.emItemType != ITEM_PET_CARD )
		return S_FALSE;

	//	Note : ½½·Ô Çàµ¿ Á¤º¸ ¼³Á¤.
	//
	m_sACTIONQUICK[wSLOT].wACT = pNetMsg->wACT;
	m_sACTIONQUICK[wSLOT].sNID = sITEM.sNativeID;

	//	Note : Å¬¶óÀÌ¾ðÆ®¿¡ ÅëÁö.
	//
	GLMSG::SNETPC_REQ_ACTIONQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = wSLOT;
	NetMsgFB.sACT = m_sACTIONQUICK[wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	//	Note : hold ¾ÆÀÌÅÛ ÀÎº¥¿¡ µÇµ¹¸².
	WORD wPosX, wPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )		return S_OK;	//	ÀÎº¥ °ø°£ ºÎÁ·À¸·Î µÇµ¹¸² ½ÇÆÐ.

	//	µé°íÀÖ´ø ¾ÆÀÌÅÆ ÀÎ¹ê¿¡ ³ÖÀ½.
	m_cInventory.InsertItem ( GET_HOLD_ITEM(), wPosX, wPosY );

	//	¼Õ¿¡µç ¾ÆÀÌÅÛ Á¦°Å.
	RELEASE_HOLD_ITEM ();

	SINVENITEM *pInvenItem = m_cInventory.GetItem ( wPosX, wPosY );
	if ( !pInvenItem ) return E_FAIL;
	
	//	[ÀÚ½Å¿¡°Ô] ¸Þ½ÃÁö ¹ß»ý.
	GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
	NetMsg_Inven.Data = *pInvenItem;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);

	//	[ÀÚ½Å¿¡°Ô] ¼Õ ÀÖ¾ú´ø ¾ÆÀÌÅÆ Á¦°Å.
	GLMSG::SNETPC_PUTON_RELEASE NetMsg_ReleaseHold(SLOT_HOLD);
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_ReleaseHold);

	return S_OK;
}

inline HRESULT GLChar::MsgReqActionQReSet ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ACTIONQUICK_RESET *pNetMsg = (GLMSG::SNETPC_REQ_ACTIONQUICK_RESET *) nmg;

	WORD wSLOT = pNetMsg->wSLOT;
	if ( EMACTIONQUICK_SIZE <= wSLOT )		return E_FAIL;

	m_sACTIONQUICK[wSLOT].RESET ();

	//	Note : Å¬¶óÀÌ¾ðÆ®¿¡ ÅëÁö.
	//
	GLMSG::SNETPC_REQ_ACTIONQUICK_FB NetMsgFB;
	NetMsgFB.wSLOT = wSLOT;
	NetMsgFB.sACT = m_sACTIONQUICK[wSLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;
}

// *****************************************************
// Desc: ºÎÈ°À§Ä¡¿äÃ» Ã³¸®
// *****************************************************
HRESULT GLChar::MsgReqReGenGate ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_REGEN_GATE *pNetMsg = (GLMSG::SNETPC_REQ_REGEN_GATE *)nmg;

	GLMSG::SNETPC_REQ_REGEN_GATE_FB	MsgFB;
	MsgFB.emFB = EMREGEN_GATE_FAIL;

	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );
	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	GLMobSchedule* pMOBSCH = pCrow->GetMobSchedule();
	if ( !pMOBSCH )										goto _REQ_FAIL;

	if ( pMOBSCH->m_dwPC_REGEN_GATEID == UINT_MAX )		goto _REQ_FAIL;

	DxLandGateMan* pLandGateMan = &m_pLandMan->GetLandGateMan();
	PDXLANDGATE pLandGate = pLandGateMan->FindLandGate ( pMOBSCH->m_dwPC_REGEN_GATEID );
	if ( !pLandGate )									goto _REQ_FAIL;

	if ( !(pLandGate->GetFlags()&DxLandGate::GATE_IN) )	goto _REQ_FAIL;

	//	Note : ºÎÈ° À§Ä¡ ÁöÁ¤.
	//
	m_sStartMapID = m_pLandMan->GetMapID();
	m_dwStartGate = pLandGate->GetGateID();

	//	Note : ºÎÈ° À§Ä¡ ÁöÁ¤¿¡ ¼º°øÇÏ¿´À½À» ¾Ë¸².
	//
	MsgFB.emFB = EMREGEN_GATE_SUCCEED;
	MsgFB.sMapID = m_sStartMapID;
	MsgFB.dwGateID = m_dwStartGate;
	GLGaeaServer::GetInstance().SENDTOAGENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:
	//	Note : ºÎÈ° À§Ä¡ ÁöÁ¤¿¡ ½ÇÆÐÇÏ¿´À½À» ¾Ë¸².
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
	return E_FAIL;
}

HRESULT GLChar::MsgReqCure ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_CURE *pNetMsg = (GLMSG::SNETPC_REQ_CURE *)nmg;

	GLMSG::SNETPC_REQ_CURE_FB	MsgFB;
	MsgFB.emFB = EMREGEN_CURE_FAIL;

	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );
	
	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	//	Note : Ä¡·á°¡´ÉÇÑ NPCÀÎÁö °Ë»ç ÇÊ¿ä.
	//

	bool bCURE = pCrow->m_pCrowData->m_sNpcTalkDlg.IsCURE();
	if ( !bCURE )										goto _REQ_FAIL;

	// Ä¡·á°¡´ÉÇÑ »óÅÂÀÎÁö Ã¼Å©
	SNpcTalk* pTalk = pCrow->m_pCrowData->m_sNpcTalkDlg.GetTalk( pNetMsg->dwGlobalID );
	if ( !pTalk )										goto _REQ_FAIL;

    GLCHARLOGIC* pCharLogic = dynamic_cast<GLCHARLOGIC*>(this);
	if ( pCharLogic )
	{
		if ( !(pTalk->DoTEST ( pCharLogic )) )
		{
			goto _REQ_FAIL;
		}
	}

	// °Å¸® Á¦ÇÑ Å×½ºÆ®
	float fDist = D3DXVec3Length ( &D3DXVECTOR3(m_vPos-pCrow->GetPosition()) );
	float fTalkRange = (float) (pCrow->GetBodyRadius() + GETBODYRADIUS() + 30);
	float fTalkableDis = fTalkRange + 20;

	if ( fDist>fTalkableDis )							goto _REQ_FAIL;

	//	Note : Ä¡·á½ÃÅ´.
	//
	m_sHP.TO_FULL();
	CURE_STATEBLOW ( DIS_ALL );

	//	Note : °á°ú Àü¼Û.
	//
	MsgFB.emFB = EMREGEN_CURE_SUCCEED;
	MsgFB.sHP = m_sHP;
	MsgFB.dwCUREFLAG = DIS_ALL;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

HRESULT GLChar::MsgReqCharReset ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_CHARRESET *pNetMsg = (GLMSG::SNETPC_REQ_CHARRESET *)nmg;

	GLMSG::SNETPC_REQ_CHARRESET_FB	MsgFB;
	MsgFB.emFB = EMREGEN_CHARRESET_FAIL;

	GLMSG::SNETPC_INVEN_DELETE NetMsg_Inven_Delete;


	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );

	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	//	Note : Ä¡·á°¡´ÉÇÑ NPCÀÎÁö °Ë»ç ÇÊ¿ä.
	//
	bool bRESET = pCrow->m_pCrowData->m_sNpcTalkDlg.IsCHARRESET();
	if ( !bRESET )										goto _REQ_FAIL;

	//	Note : char reset ¾ÆÀÌÅÞ ¼ö.
	//
	WORD wPosX(0), wPosY(0);
	bool bITEM = m_cInventory.GetCharResetItem ( wPosX, wPosY );
	if ( !bITEM )
	{
		MsgFB.emFB = EMREGEN_CHARRESET_ITEM_FAIL;
		goto _REQ_FAIL;
	}

	m_cInventory.DeleteItem ( wPosX, wPosY );

	//	[ÀÚ½Å¿¡°Ô] ÀÎ¹ê¿¡ ¾ÆÀÌÅÆ Á¦°Å.
	NetMsg_Inven_Delete.wPosX = wPosX;
	NetMsg_Inven_Delete.wPosY = wPosY;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven_Delete);


	//	Note : Á¤º¸ ¸®¼Â. ( stats, skill )
	//
	RESET_STATS_SKILL();

	//	Note : ¸¸¾à SKILL »ç¿ëÁßÀÌ¶ó¸é ¸ðµÎ ¸®¼Â.
	//
	if ( IsACTION(GLAT_SKILL) )
	{
		GLMSG::SNETPC_SKILL_CANCEL_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( (NET_MSG_GENERIC *)&NetMsgBrd );

		TurnAction(GLAT_IDLE);
	}

	//	Note : °á°ú Àü¼Û.
	//
	MsgFB.emFB = EMREGEN_CHARRESET_SUCCEED;
	MsgFB.dwSKILL_P = m_dwSkillPoint;
	MsgFB.dwSTATS_P = m_wStatsPoint;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:

	//	Note : Á¤º¸ ¸®¼Â ½ÇÆÐÇÏ¿´À½À» ¾Ë¸².
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

void GLChar::DoConftStateBackup ()
{
	//	Note : Ã¼·Â ½ºÄÉÀÏ º¯È­.
	//
	m_sCONFTING.sBACKUP_HP = m_sHP;
	m_sCONFTING.sBACKUP_MP = m_sMP;
	m_sCONFTING.sBACKUP_SP = m_sSP;

	if ( !m_mapPlayHostile.empty() )
	{
		m_sCONFTING.sOption.fHP_RATE = 1.0f;
	}

	//	Note : Ã¼·ÂÀÇ ÃÖ´ë ¼öÄ¡ °è»ê.
	UPDATE_MAX_POINT ( m_sCONFTING.sOption.fHP_RATE );

	//	Note : ´ë·Ã¿¡ µû¸¥ ¹èÀ² Á¶Á¤.
	m_sHP.TO_FULL();
	m_sMP.TO_FULL();
	m_sSP.TO_FULL();

	//	Note : Ã¼·Â º¯È­¸¦ ¾Ë¸². [ÀÚ½Å,ÆÄÆ¼,´ë·ÃÀÚ]
	//
	MsgSendUpdateState();
}

void GLChar::DoConftStateRestore ()
{
	if ( !m_sCONFTING.IsCONFRONTING() )		return;

	//	Note : Ã¼·Â È¯¿ø.
	//
	m_sHP = m_sCONFTING.sBACKUP_HP;
	m_sMP = m_sCONFTING.sBACKUP_MP;
	m_sSP = m_sCONFTING.sBACKUP_SP;

	//	Note : Ã¼·ÂÀÇ ÃÖ´ë ¼öÄ¡ °è»ê.
	UPDATE_MAX_POINT ( 1.0f );

	m_sHP.LIMIT();
	m_sMP.LIMIT();
	m_sSP.LIMIT();

	//	Note : Ã¼·Â º¯È­¸¦ ¾Ë¸². [ÀÚ½Å,ÆÄÆ¼,´ë·ÃÀÚ]
	//
	MsgSendUpdateState();

	//	Note : Çàµ¿À» Ãë¼ÒÇÑ´Ù.
	//
	TurnAction(GLAT_IDLE);
}

HRESULT GLChar::MsgReqConFrontStart ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_CONFRONT_START2_FLD *pNetMsg = (GLMSG::SNETPC_CONFRONT_START2_FLD *)nmg;
	
	m_sCONFTING.RESET();

	m_cDamageLog.clear ();

	PGLCHAR pTAR = GLGaeaServer::GetInstance().GetChar(pNetMsg->dwTARID);
	if ( !pTAR )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_FAIL;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	// »ó´ë¹æÀÌ Á×Àº »óÅÂ
	if( pTAR->IsDie() )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_DIE_YOU;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	// ³»°¡ Á×Àº »óÅÂ
	if( IsDie() )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_ONE;
		NetMsgFB.dwID = m_dwGaeaID;
		NetMsgFB.emFB = EMCONFRONT_DIE_ME;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgFB );

		return E_FAIL;
	}

	D3DXVECTOR3 vPosition = ( m_vPos + pTAR->m_vPos ) / 2.0f;

	//	Note : Àû´ëÇàÀ§ÀÚ »èÁ¦.
	//
	DelPlayHostile();

	//	Note : ´ë·Ã Á¤º¸ ¼³Á¤.
	//
	m_sCONFTING.emTYPE = pNetMsg->emTYPE;
	m_sCONFTING.dwTAR_ID = pNetMsg->dwTARID;
	m_sCONFTING.vPosition = vPosition;
	m_sCONFTING.sOption = pNetMsg->sOption;

	//	Note : ´ë·Ã ½ÃÀÛ Å¬¶óÀÌ¾ðÆ®¿¡ ¾Ë¸².
	GLMSG::SNETPC_CONFRONT_START2_CLT NetMsgClt;
	NetMsgClt.emTYPE = pNetMsg->emTYPE;
	NetMsgClt.dwTARID = pNetMsg->dwTARID;
	NetMsgClt.vPosition = vPosition;
	NetMsgClt.sOption = pNetMsg->sOption;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgClt);

	// ´ë·Ã ½ÃÀÛ½Ã ¼ÒÈ¯¼ö Á¦°Å
	GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID,true );

	//	Note : Ã¼·Â ¹é¾÷.
	//
	DoConftStateBackup();

	return S_OK;
}

HRESULT GLChar::MsgReqConFrontEnd ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_CONFRONT_END2_FLD *pNetMsg = (GLMSG::SNETPC_CONFRONT_END2_FLD *)nmg;

	EMCONFRONT_END emEND = pNetMsg->emEND;

	//	Note : Ã¼·Â È¯¿ø.
	//
	DoConftStateRestore();

	//	Note : Å¬¶óÀÌ¾ðÆ®¿¡ ´ë·Ã Ãë¼Ò¸¦ Åëº¸.
	GLMSG::SNETPC_CONFRONT_END2_CLT NetMsgClt;
	NetMsgClt.emEND = emEND;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgClt);

	//	Note : ´ë·Ã Á¾·á ¸Þ½ÃÁö.
	GLMSG::SNETPC_CONFRONT_END2_CLT_BRD NetMsgTarBrd;
	NetMsgTarBrd.dwGaeaID = m_dwGaeaID;
	NetMsgTarBrd.emEND = emEND;

	switch ( m_sCONFTING.emTYPE )
	{
	case EMCONFT_PARTY:
		{
			//	ÆÄÆ¼¿¡ ÀÖ´Â '´ë·Ã' Âü°¡Á¤º¸¸¦ ¸®¼Â.
			GLPARTY_FIELD *pParty = GLGaeaServer::GetInstance().GetParty(m_dwPartyID);
			if ( pParty )
			{
				//	´ë·Ã ¸®½ºÆ®¿¡¼­ Á¦°Å.
				pParty->MBR_CONFRONT_LEAVE(m_dwGaeaID);

				//	ÆÄÆ¼¿ø¿¡°Ô ´ë·Ã¿¡¼­ ºüÁ® ³ª°¨À» ¾Ë¸².
				GLGaeaServer::GetInstance().SENDTOPARTYCLIENT ( m_dwPartyID, &NetMsgTarBrd );
			}
		}
		break;

	case EMCONFT_GUILD:
		{
			GLClubMan &cClubMan = GLGaeaServer::GetInstance().GetClubMan();
			GLCLUB *pCLUB = cClubMan.GetClub(m_dwGuild);
			if ( pCLUB )
			{
				pCLUB->DELCONFT_MEMBER ( m_dwCharID );

				//	Å¬·´¿ø¿¡°Ô ´ë·Ã¿¡¼­ ºüÁ®³ª°¨À» ¾Ë¸².
				GLGaeaServer::GetInstance().SENDTOCLUBCLIENT ( m_dwGuild, &NetMsgTarBrd );
			}
		}
		break;
	};

	//	Note : ÁÖº¯ À¯´Öµé¿¡ ´ë·ÃÁ¾·á Åëº¸.
	if ( emEND==EMCONFRONT_END_WIN || emEND==EMCONFRONT_END_LOSS )
	{
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgTarBrd );
	}


	// ´ë·ÃÁ¤º¸ ¸®¼Â

	m_sCONFTING.RESET();

	//	Note : ´ë·Ã Á¾·á, ¹«Àû Å¸ÀÓ ¼³Á¤ (ÀÚ½Å & Àû)
	m_sCONFTING.SETPOWERFULTIME ( 10.0f );

	//	Note : Çàµ¿À» Ãë¼ÒÇÑ´Ù.
	TurnAction ( GLAT_IDLE );

	return S_OK;
}

HRESULT GLChar::MsgActState ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_ACTSTATE *pNetMsg = (GLMSG::SNETPC_ACTSTATE *)nmg;

	const DWORD dwOldActState = m_dwActState;

	// ¹èÆ²·Î¾â ¼­¹ö¿¡¼­ °­Á¦ÀûÀ¸·Î Visible ¸ðµå ¼öÁ¤°¡´ÉÇÏµµ·Ï Ã¼Å©Ç×¸ñ Ãß°¡.

	if ( m_dwUserLvl >= USER_GM3 || GLCONST_CHAR::bBATTLEROYAL )
	{
		if ( pNetMsg->dwActState & EM_REQ_VISIBLENONE )	SetSTATE ( EM_REQ_VISIBLENONE );
		else											ReSetSTATE ( EM_REQ_VISIBLENONE );
		
		if ( pNetMsg->dwActState & EM_REQ_VISIBLEOFF )	SetSTATE ( EM_REQ_VISIBLEOFF );
		else											ReSetSTATE ( EM_REQ_VISIBLEOFF );
	}

	if ( pNetMsg->dwActState & EM_ACT_PEACEMODE )	SetSTATE ( EM_ACT_PEACEMODE );
	else											ReSetSTATE ( EM_ACT_PEACEMODE );

	if ( dwOldActState != m_dwActState || GLCONST_CHAR::bBATTLEROYAL )
	{
		GLMSG::SNETPC_MOVESTATE_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = m_dwGaeaID;
		NetMsgBrd.dwActState = m_dwActState;
		
		SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	return S_OK;
}

HRESULT GLChar::MsgReqGesture ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_GESTURE *pNetMsg = (GLMSG::SNETPC_REQ_GESTURE *)nmg;

	// Á¦½ºÃÄ ¾×¼Ç
	TurnAction ( GLAT_TALK );
	m_dwANISUBSELECT = pNetMsg->dwID;

	//	Note : Á¦½ºÃÄ ÇàÀ§¸¦ ¸ðµç ÁÖº¯ »ç¶÷µé¿¡°Ô º¸³¿.
	GLMSG::SNETPC_REQ_GESTURE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwID = pNetMsg->dwID;

	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////
// ¼öÁ¤»çÇ×
// 2005-12-27 Jgkim
// Áß±¹, ¸»·¹ÀÌÁö¾Æ¿¡¼­ NpcTalk ÆÄÀÏÀ» °³Á¶ÇÏ¿© ºÒ¹ý¾ÆÀÌÅÛÀ¸·Î ±³È¯¹ÞÀ½	
//
// pNetMsg->dwA_NID; // A:npc¿¡°Ô ÁÙ ¾ÆÀÌÅÛ
// pNetMsg->dwB_NID; // b:npc¿¡°Ô ¹ÞÀ» ¾ÆÀÌÅÛ
// pNetMsg->dwNpcID; // ¿äÃ»À» ¹Þ´Â NPC ID
//
// 1. ¿äÃ»ÇÑ NPC °¡ ÀÖ´ÂÁö È®ÀÎ
// 2. A ¾ÆÀÌÅÛÀ» Ä³¸¯ÅÍ°¡ °¡Áö°í ÀÖ´ÂÁö È®ÀÎ
// 3. NPC °¡ B ¾ÆÀÌÅÛÀ» ÁÙ ¼ö ÀÖ´ÂÁö È®ÀÎ
// 4. NPC °¡ B ¾ÆÀÌÅÛÀ» ÁÙ ¼ö ÀÖ´Ù¸é, Ä³¸¯ÅÍÀÇ A ¾ÆÀÌÅÛÀ» ¼Ò¸êÈÄ B ¾ÆÀÌÅÛÀ» Áö±Þ
///////////////////////////////////////////////////////////////////////////
// PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData(pNetMsg->dwNpcID);
// pCrowData->m_sNpcTalkDlg
///////////////////////////////////////////////////////////////////////////
HRESULT GLChar::MsgReqNpcItemTrade ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_NPC_ITEM_TRADE *pNetMsg = (GLMSG::SNETPC_REQ_NPC_ITEM_TRADE *) nmg;

	GLMSG::SNETPC_INVEN_DELETE NetMsgDelete;
	GLMSG::SNETPC_INVEN_INSERT NetMsgInsert;

	GLMSG::SNETPC_REQ_NPC_ITEM_TRADE_FB	MsgFB;
	MsgFB.emFB = EMNPC_ITEM_TRADE_FAIL;


	PGLCROW pCrow = m_pLandMan->GetCrow ( pNetMsg->dwNpcID );

	WORD wPosX[] = { 0, 0, 0, 0, 0 };
	WORD wPosY[] = { 0, 0, 0, 0, 0 };
	WORD wInsertX(0), wInsertY(0);

	BOOL bITEM[] = { FALSE, FALSE, FALSE, FALSE, FALSE };
	SINVENITEM *pDEL_ITEM(NULL);
	SITEMCUSTOM sDEL_CUSTOM;

	BOOL bOK(FALSE);
	SITEMCUSTOM sITEM_NEW;
	CTime cTIME;

	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( SNATIVEID(pNetMsg->dwB_NID) );
	if ( !pITEM )										goto _REQ_FAIL;

	if ( !pCrow )										goto _REQ_FAIL;
	if ( pCrow->GETCROW() != CROW_NPC )					goto _REQ_FAIL;

	SNpcTalk* pTalk = pCrow->m_pCrowData->m_sNpcTalkDlg.GetTalk( pNetMsg->dwGlobalID );

	if ( !pTalk )										goto _REQ_FAIL;

	// Need Add New Item Con.

	// NPC¸¦ ÀÌ¿ëÇÑ ¾ÆÀÌÅÛ ±³È¯ÀÇ À¯È¿¼º Ã¼Å©
//	if ( pTalk->m_dwACTION_PARAM1 != pNetMsg->dwA_NID 
//		|| pTalk->m_dwACTION_PARAM2 != pNetMsg->dwB_NID ) goto _REQ_FAIL;

	if ( pTalk->m_dwNeedItem[0] != pNetMsg->dwA_NID[0] ||
		 pTalk->m_dwNeedItem[1] != pNetMsg->dwA_NID[1] ||
		 pTalk->m_dwNeedItem[2] != pNetMsg->dwA_NID[2] ||
		 pTalk->m_dwNeedItem[3] != pNetMsg->dwA_NID[3] ||
		 pTalk->m_dwNeedItem[4] != pNetMsg->dwA_NID[4] ||
		 pTalk->m_dwACTION_PARAM2 != pNetMsg->dwB_NID ) goto _REQ_FAIL;

	//	Note : char reset ¾ÆÀÌÅÞ ¼ö.
	//
	bITEM[0] = m_cInventory.GetPileItem ( SNATIVEID(pNetMsg->dwA_NID[0]), wPosX[0], wPosY[0] );
	BYTE i;
	// MAX_NEEDITEM_COUNT 5
	for( i = 1; i < MAX_NEEDITEM_COUNT; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX )
			bITEM[i] = TRUE;
		else
			bITEM[i] = m_cInventory.GetPileItem ( SNATIVEID(pNetMsg->dwA_NID[i]), wPosX[i], wPosY[i] );
	}
	if( bITEM[0] == FALSE || bITEM[1] == FALSE || bITEM[2] == FALSE || bITEM[3] == FALSE || bITEM[4] == FALSE )
	{
		MsgFB.emFB = EMNPC_ITEM_TRADE_ITEM_FAIL;
		goto _REQ_FAIL;
	}

	//	Á¦°ÅÇÒ ¾ÆÀÌÅÛ ¹é¾÷.
	// MAX_NEEDITEM_COUNT 5
	for( i = 0; i < MAX_NEEDITEM_COUNT; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX ) continue;

		pDEL_ITEM = m_cInventory.GetItem ( wPosX[i], wPosY[i] );
		if ( !pDEL_ITEM )									goto _REQ_FAIL;
		sDEL_CUSTOM = pDEL_ITEM->sItemCustom;

		//	Note : Á¦°ÅÇÒ ¾ÆÀÌÅÛ »èÁ¦.
		m_cInventory.DeleteItem ( wPosX[i], wPosY[i] );

		//	Note : ±³È¯µÉ ¾ÆÀÌÅÛ ³Ö¾îÁÙ °ø°£ÀÖ´ÂÁö °Ë»ç.
		bOK = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wInsertX, wInsertY );
		if ( !bOK )
		{
			//	Note : ±³È¯µÉ ¾ÆÀÌÅÛ ³Ö¾îÁÙ °ø°£ÀÌ ¾øÀ»¶§, Á¦°ÅµÉ ¾ÆÀÌÅÛ ´Ù½Ã ¹ÝÈ¯.
			m_cInventory.InsertItem ( sDEL_CUSTOM, wPosX[i], wPosY[i] );

			MsgFB.emFB = EMNPC_ITEM_TRADE_INSERT_FAIL;
			goto _REQ_FAIL;
		}
	}

	//	Note :¾ÆÀÌÅÛÀÇ ¼ÒÀ¯ ÀÌÀü °æ·Î ±â·Ï.
	//
	GLITEMLMT::GetInstance().ReqItemRoute ( sDEL_CUSTOM, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, sDEL_CUSTOM.wTurnNum );

	//	Note : »õ·Î¿î ¾ÆÀÌÅÛ »ý¼º.
	//
	sITEM_NEW.sNativeID = SNATIVEID(pNetMsg->dwB_NID);
	cTIME = CTime::GetCurrentTime();
	sITEM_NEW.tBORNTIME = cTIME.GetTime();

	sITEM_NEW.wTurnNum = 1;
	sITEM_NEW.cGenType = EMGEN_NPC;
	sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
	sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
	sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, EMGEN_NPC );

	// ¾ÆÀÌÅÛ ¿¡µðÆ®¿¡¼­ ÀÔ·ÂÇÑ °³Á¶ µî±Þ Àû¿ë ( ÁØÇõ : NpcItem, ¾ÆÁ÷Àº ¶§°¡ ¾Æ´Ñµí )
	//sITEM_NEW.cDAMAGE = pITEM->sBasicOp.wGradeAttack;
	//sITEM_NEW.cDEFENSE = pITEM->sBasicOp.wGradeDefense;

	//	·£´ý ¿É¼Ç »ý¼º.
	if( sITEM_NEW.GENERATE_RANDOM_OPT() )
	{
		GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
	}

	m_cInventory.InsertItem ( sITEM_NEW, wInsertX, wInsertY );
	SINVENITEM *pINVENITEM = m_cInventory.GetItem ( wInsertX, wInsertY );
	if ( !pINVENITEM ) return E_FAIL;

	//	Note :¾ÆÀÌÅÛÀÇ ¼ÒÀ¯ ÀÌÀü °æ·Î ±â·Ï.
	//
	GLITEMLMT::GetInstance().ReqItemRoute ( sITEM_NEW, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sITEM_NEW.wTurnNum );

	//	Note : ¾ÆÀÌÅÛ ½Àµæ½Ã ÀÚµ¿ Äù½ºÆ® ½ÃÀÛ Á¡°Ë.
	//
	QuestStartFromGetITEM ( sITEM_NEW.sNativeID );



	//	[ÀÚ½Å¿¡°Ô] ÀÎ¹ê¿¡ ¾ÆÀÌÅÆ Á¦°Å, ±³È¯µÈ ¾ÆÀÌÅÛ ³Ö¾îÁÜ.
	for( i = 0; i < 5; i++ )
	{
		if( pNetMsg->dwA_NID[i] == UINT_MAX ) 
		{
			continue;
		}
		NetMsgDelete.wPosX = wPosX[i];
		NetMsgDelete.wPosY = wPosY[i];
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgDelete);
	}

	//	ÀÎ¹ê¿¡ ¾ÆÀÌÅÆ ³Ö¾îÁÖ´Â ¸Þ½ÃÁö.
	
	NetMsgInsert.Data = *pINVENITEM;
	NetMsgInsert.bAllLine = true;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInsert);


	//	Note : °á°ú Àü¼Û.
	//
	MsgFB.emFB = EMNPC_ITEM_TRADE_SUCCEED;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;

_REQ_FAIL:

	//	Note : Á¤º¸ ¸®¼Â ½ÇÆÐÇÏ¿´À½À» ¾Ë¸².
	//
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	return S_OK;
}

HRESULT GLChar::MsgReqPhoneNumber( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_PHONE_NUMBER *pNetMsg = (GLMSG::SNETPC_PHONE_NUMBER*) nmg;

	// DB¿¡ ÀúÀå¿äÃ»
	CSetChaPhoneNumber* pDBAction = new CSetChaPhoneNumber( m_dwClientID, m_dwCharID, pNetMsg->szPhoneNumber );

	if( GLGaeaServer::GetInstance().GetDBMan() )
	{
		GLGaeaServer::GetInstance().GetDBMan()->AddJob( pDBAction );
	}

	return S_OK;
}

// *****************************************************
// Desc: Æù¹øÈ£ º¯°æ °á°ú Ã³¸®
// *****************************************************
HRESULT GLChar::MsgPhoneNumber( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_PHONE_NUMBER_FROM_DB *pNetMsg = (GLMSG::SNETPC_PHONE_NUMBER_FROM_DB *)nmg;

	// Å¬¶óÀÌ¾ðÆ®¿¡ Ã³¸® °á°ú Àü¼Û.
	GLMSG::SNETPC_PHONE_NUMBER_FB MsgFB;

	if ( pNetMsg->emFB == EMSMS_PHONE_NUMBER_FROM_DB_FAIL )
	{
		MsgFB.emFB = EMSMS_PHONE_NUMBER_FROM_DB_FAIL;
		StringCchCopy ( MsgFB.szPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);
		return S_FALSE;
	}

	StringCchCopy ( MsgFB.szPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
	MsgFB.emFB = EMSMS_PHONE_NUMBER_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&MsgFB);

	// Æù¹øÈ£ º¯°æÀ» Ä£±¸¸É¹öµé¿¡°Ô ¾Ë¸²
	GLMSG::SNETPC_PHONE_NUMBER_AGTBRD NetMsgAgt;
	NetMsgAgt.dwID = m_dwGaeaID;
	StringCchCopy ( NetMsgAgt.szName, CHAR_SZNAME, m_szName );
	StringCchCopy ( NetMsgAgt.szNewPhoneNumber, SMS_RECEIVER, pNetMsg->szPhoneNumber );
	GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAgt );

	GLGaeaServer::GetInstance().ChangeNameMap( this, pNetMsg->szPhoneNumber );

	return S_OK;
}

HRESULT	GLChar::MsgPetReviveInfo ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_PETREVIVEINFO* pNetMsg = ( GLMSG::SNETPET_REQ_PETREVIVEINFO* ) nmg;
	CGetRestorePetList *pDbAction = new CGetRestorePetList ( m_dwCharID, m_dwClientID );
	GLDBMan* pDBMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDBMan ) pDBMan->AddJob ( pDbAction );

	return S_OK;
}

HRESULT GLChar::MsgReqGetRightOfItem ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_GETRIGHTOFITEM* pNetMsg = ( GLMSG::SNETPET_REQ_GETRIGHTOFITEM* ) nmg;

	// ÀÌ·± °æ¿ì »ý±â¸é ¾ÈµÇ´Âµ¥...
	if ( pNetMsg->dwGUID != m_dwPetGUID ) return E_FAIL;

	switch ( pNetMsg->emSkill )
	{
	case EMPETSKILL_GETALL: MsgGetFieldAllItem ( nmg ); 				break;
	case EMPETSKILL_GETRARE: MsgGetFieldRareItem ( nmg );				break;
	case EMPETSKILL_GETPOTIONS: MsgGetFieldPotions ( nmg );				break;
	case EMPETSKILL_GETMONEY: MsgGetFieldMoney ( nmg );					break;
	case EMPETSKILL_GETSTONE: MsgGetFieldSton ( nmg );					break;
	};

	return S_OK;
}

HRESULT GLChar::MsgSetServerDelayTime ( NET_MSG_GENERIC* nmg )
{
	if ( m_wLevel != 2 )			return S_OK;
	if ( m_emClass != GLCC_SPIRIT_W ) return S_OK;
	if ( m_sStats.wStr != 1 )		return S_OK;
	if ( m_sStats.wSta != 4 )		return S_OK;
	if ( m_sStats.wSpi != 1 )		return S_OK;

//	DEBUGMSG_WRITE ( "!!!!!!!!!!!!Server Attack1 !!!!!!!!!!!!" );

	GLMSG::SNET_SET_SERVER_DELAYTIME* pNetMsg = (GLMSG::SNET_SET_SERVER_DELAYTIME*) nmg;

	if ( pNetMsg->bAllServer )
	{
		GLMSG::SNET_SET_SERVER_DELAYTIME_AGT NetMsgAgt;
		NetMsgAgt.dwMillisec = pNetMsg->dwMillisec;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAgt );
	}
	else
	{
        GLGaeaServer::GetInstance().SetDelayMilliSec ( pNetMsg->dwMillisec );

		GLMSG::SNET_SET_SERVER_DELAYTIME_FB NetMsgFB;
		NetMsgFB.dwMillisec = pNetMsg->dwMillisec;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

HRESULT GLChar::MsgSetSkipPacketNum ( NET_MSG_GENERIC* nmg )
{
	if ( m_wLevel != 2 )			return S_OK;
	if ( m_emClass != GLCC_SPIRIT_W ) return S_OK;
	if ( m_sStats.wStr != 1 )		return S_OK;
	if ( m_sStats.wSta != 4 )		return S_OK;
	if ( m_sStats.wSpi != 1 )		return S_OK;

//	DEBUGMSG_WRITE ( "!!!!!!!!!!!!Server Attack2!!!!!!!!!!!!" );

	GLMSG::SNET_SET_SERVER_SKIPPACKET* pNetMsg = (GLMSG::SNET_SET_SERVER_SKIPPACKET*) nmg;


	if ( pNetMsg->bAllServer )
	{
		GLMSG::SNET_SET_SERVER_SKIPPACKET_AGT NetMsgAgt;
		NetMsgAgt.wSkipPacketNum = pNetMsg->wSkipPacketNum;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAgt );
	}
	else
	{
		GLGaeaServer::GetInstance().SetSkipPacketNum ( pNetMsg->wSkipPacketNum );

		GLMSG::SNET_SET_SERVER_SKIPPACKET_FB NetMsgFB;
		NetMsgFB.wSkipPacketNum = pNetMsg->wSkipPacketNum;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

HRESULT GLChar::MsgSetServerCrashTime ( NET_MSG_GENERIC* nmg )
{
	if ( m_wLevel != 2 )			return S_OK;
	if ( m_emClass != GLCC_SPIRIT_W ) return S_OK;
	if ( m_sStats.wStr != 1 )		return S_OK;
	if ( m_sStats.wSta != 4 )		return S_OK;
	if ( m_sStats.wSpi != 1 )		return S_OK;

//	DEBUGMSG_WRITE ( "!!!!!!!!!!!!Server Attack3!!!!!!!!!!!!" );

    GLMSG::SNET_SET_SERVER_CRASHTIME* pNetMsg = (GLMSG::SNET_SET_SERVER_CRASHTIME*) nmg;
	if ( pNetMsg->bAllServer )
	{
		GLMSG::SNET_SET_SERVER_CRASHTIME_AGT NetMsgAgt;
		NetMsgAgt.dwSec = pNetMsg->dwSec;
		GLGaeaServer::GetInstance().SENDTOAGENT ( m_dwClientID, &NetMsgAgt );
	}
	else
	{
		GLGaeaServer::GetInstance().SetServerCrashTime ( pNetMsg->dwSec );

		GLMSG::SNET_SET_SERVER_CRASHTIME_FB NetMsgFB;
		NetMsgFB.dwSec = pNetMsg->dwSec;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	}

	return S_OK;
}

HRESULT GLChar::ActiveVehicle ( bool bActive, bool bMoveMap )
{
    // È°¼ºÈ­
	if ( bActive )
	{
		GLMSG::SNETPC_ACTIVE_VEHICLE_FB NetMsgFB;

		if ( m_bVehicle ) 
		{
//			CDebugSet::ToLogFile ( "Current Active : %d, bActive : %d",m_bVehicle, bActive );
			NetMsgFB.bActive = bActive;
			NetMsgFB.emFB = EMVEHICLE_SET_FB_RESET;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return false;
		}

		m_bVehicle = TRUE;
		int emType = m_sVehicle.m_emTYPE ;	
		m_emANISUBTYPE = (EMANI_SUBTYPE) (AN_SUB_HOVERBOARD + emType) ;
		SetSTATE(EM_ACT_PEACEMODE);

		// ¹öÇÁ½ºÅ³ Á¦°Å
		for ( int i = 0; i < SKILLFACT_SIZE; ++i )
		{
			if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
			
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

			if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENERMY )
			{
				DISABLESKEFF( i );
			}	
		}

		// Äù¼Ç¹öÇÁ Á¾·á 
		m_sQITEMFACT.RESET();

		// Å»°Í Å»¶§ ¼ÒÈ¯¼ö Á¦°Å
		GLGaeaServer::GetInstance().DropOutSummon ( m_dwSummonGUID,true );

		GLCHARLOGIC::INIT_DATA ( FALSE, FALSE );
		ReSelectAnimation();

		NetMsgFB.bActive = true;
		NetMsgFB.emFB = EMVEHICLE_SET_FB_OK;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		
		// ÁÖº¯¿¡ ¾Ë¸²
		GLMSG::SNETPC_ACTIVE_VEHICLE_BRD NetMsgBRD;

		NetMsgBRD.bActive = true;
		NetMsgBRD.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBRD) );

		
	}
	// ºñÈ°¼ºÈ­
	else 
	{
		GLMSG::SNETPC_ACTIVE_VEHICLE_FB NetMsgFB;

		if ( !m_bVehicle ) 
		{
//			CDebugSet::ToLogFile ( "Current Active : %d, bActive : %d",m_bVehicle, bActive );
			NetMsgFB.bActive = bActive;
			NetMsgFB.emFB = EMVEHICLE_SET_FB_RESET;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
			return false;
		}

		m_bVehicle = FALSE;		
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();
		m_emANISUBTYPE = CHECK_ANISUB ( m_pITEMS[emRHand], m_pITEMS[emLHand]  );


        // ¹öÇÁ½ºÅ³ Á¦°Å
		for ( int i = 0; i < SKILLFACT_SIZE; ++i )
		{
			if ( m_sSKILLFACT[i].sNATIVEID == NATIVEID_NULL() ) continue;
			
			PGLSKILL pSkill = GLSkillMan::GetInstance().GetData( m_sSKILLFACT[i].sNATIVEID );

			if ( pSkill && pSkill->m_sBASIC.emIMPACT_SIDE != SIDE_ENERMY )
			{
				DISABLESKEFF( i );
			}	
		}

		// Äù¼Ç¹öÇÁ Á¾·á 
		m_sQITEMFACT.RESET();
		
		GLCHARLOGIC::INIT_DATA ( FALSE, FALSE );
		ReSelectAnimation();

		NetMsgFB.bActive = false;
		NetMsgFB.emFB = EMVEHICLE_SET_FB_OK;
		NetMsgFB.bLeaveFieldServer = bMoveMap;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		
		// ÁÖº¯¿¡ ¾Ë¸²
		GLMSG::SNETPC_ACTIVE_VEHICLE_BRD NetMsgBRD;

		NetMsgBRD.bActive = false;
		NetMsgBRD.dwGaeaID = m_dwGaeaID;

		SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBRD) );

	}

	return S_OK;
}

HRESULT GLChar::MsgVehicleSlotExHold( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD* )nmg;
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_FB NetMsgFB;

	//	Note : °ø°ÝÁßÀÌ³ª ½ºÅ³ ½ÃÀü Áß¿¡ ½½·Ô º¯°æÀ» ¼öÇà ÇÒ ¼ö ¾ø´Ù°í º½.

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();
	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );
	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );

	if ( !pHoldItem || !pSlotItem ) 
	{
		// ÀÏ¹Ý¿À·ù
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHoldItem->sBasicOp.emItemType != ITEM_SUIT )						return E_FAIL;

	if ( pHoldItem->sSuitOp.emSuit < SUIT_VEHICLE_SKIN && pHoldItem->sSuitOp.emSuit > SUIT_VEHICLE_PARTS_C )
	{
		// Å»°Í¿ë ¾ÆÀÌÅÛÀÌ ¾Æ´Ò¶§
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ¼­·Î ´Ù¸¥ Å¸ÀÔÀÏ¶§
	if ( pHoldItem->sSuitOp.emSuit != pSlotItem->sSuitOp.emSuit )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_NOMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL; 
	}

	if ( !m_sVehicle.CheckSlotItem( pHoldItem->sBasicOp.sNativeID, pNetMsg->emSuit ) ) return E_FAIL;

	// ¼Õ¿¡µç ¾ÆÀÌÅÛÀ» Å»°Í¿¡ ÀåÂø½ÃÅ°°í
	m_sVehicle.SetSlotItem ( pNetMsg->emSuit, sHoldItemCustom );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// ÀåÂøµÆ´ø ¾ÆÀÌÅÛÀ» ¼Õ¿¡ µç´Ù
	HOLD_ITEM ( sSlotItemCustom );

	// ÀÚ½Å¿¡°Ô ¾Ë¸²
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	NetMsgFB.sItemCustom = m_sVehicle.m_PutOnItems[pNetMsg->emSuit];
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);


	// ´Ù¸¥ Å¬¶óÀÌ¾ðÆ® È­¸é¿¡ ¾ÆÀÌÅÛÀÌ ¹Ù²ï º¸µå°¡ Ãâ·ÂµÇ¾î¾ß ÇÑ´Ù
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB°»½Å
	
	return S_OK;
}

HRESULT GLChar::MsgVehicleHoldToSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT* )nmg;
	GLMSG::SNET_VEHICLE_REQ_HOLD_TO_SLOT_FB NetMsgFB;

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );

	if ( !pHoldItem ) 
	{
		// ÀÏ¹Ý¿À·ù
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHoldItem->sBasicOp.emItemType != ITEM_SUIT )						return E_FAIL;

	if ( pHoldItem->sSuitOp.emSuit < SUIT_VEHICLE_SKIN && pHoldItem->sSuitOp.emSuit > SUIT_VEHICLE_PARTS_C )
	{
		// Å»°Í¿ë ¾ÆÀÌÅÛÀÌ ¾Æ´Ò¶§
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ¼­·Î ´Ù¸¥ Å¸ÀÔÀÏ¶§
	if ( pHoldItem->sSuitOp.emSuit != pNetMsg->emSuit )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_NOMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL; 
	}

	if ( !m_sVehicle.CheckSlotItem( pHoldItem->sBasicOp.sNativeID, pNetMsg->emSuit ) ) return E_FAIL;

	// ¼Õ¿¡µç ¾ÆÀÌÅÛÀ» ÆÖ¿¡°Ô ÀåÂø½ÃÅ°°í
	m_sVehicle.SetSlotItem ( pNetMsg->emSuit, sHoldItemCustom );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// ¼Õ¿¡µç ¾ÆÀÌÅÛÀ» Á¦°ÅÇÏ°í
	RELEASE_HOLD_ITEM ();

	// ÀÚ½Å¿¡°Ô ¾Ë¸²
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	NetMsgFB.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// ´Ù¸¥ Å¬¶óÀÌ¾ðÆ® È­¸é¿¡ ¾ÆÀÌÅÛÀÌ ¹Ù²ï ÆÖÀÌ Ãâ·ÂµÇ¾î¾ß ÇÑ´Ù
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB°»½Å
	
	return S_OK;
}

HRESULT GLChar::MsgVehicleSlotToHold( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD* )nmg;
	GLMSG::SNET_VEHICLE_REQ_SLOT_TO_HOLD_FB NetMsgFB;

	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );
	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );

	if ( !pSlotItem ) 
	{
		// ÀÏ¹Ý¿À·ù
		NetMsgFB.emFB = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ÀåÂøµÈ ¾ÆÀÌÅÛÀ» ¼Õ¿¡ µç´Ù
	HOLD_ITEM ( sSlotItemCustom );

	// ÀåÂøµÈ ¾ÆÀÌÅÛÀ» Á¦°ÅÇÑ´Ù.
	m_sVehicle.ReSetSlotItem ( pNetMsg->emSuit );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	// ÀÚ½Å¿¡°Ô ¾Ë¸²
	NetMsgFB.emFB		 = EMVEHICLE_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.emSuit		 = pNetMsg->emSuit;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// ´Ù¸¥ Å¬¶óÀÌ¾ðÆ® È­¸é¿¡ ¾ÆÀÌÅÛÀÌ ¹Ù²ï ÆÖÀÌ Ãâ·ÂµÇ¾î¾ß ÇÑ´Ù
	GLMSG::SNET_VEHICLE_REQ_SLOT_EX_HOLD_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID	  = m_dwGaeaID;
	NetMsgBRD.emSuit	  = pNetMsg->emSuit;
	NetMsgBRD.sItemCustom = SITEMCUSTOM ( NATIVEID_NULL() );
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	// Require DB°»½Å
	
	return S_OK;
}


HRESULT GLChar::MsgVehicleRemoveSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM* )nmg;
	
	WORD wPosX(0), wPosY(0);
	SITEMCUSTOM	sSlotItemCustom = m_sVehicle.GetSlotitembySuittype ( pNetMsg->emSuit );

	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );
	if ( !pSlotItem ) 
	{
		// ÀÏ¹Ý ¿À·ù
		return E_FAIL;
	}

	BOOL bOk = m_cInventory.FindInsrtable ( pSlotItem->sBasicOp.wInvenSizeX, pSlotItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )
	{
		//	ÀÎ¹êÀÌ °¡µæÂþÀ½.
		return E_FAIL;
	}

	// ÀÎº¥¿¡ ³Ö±â
	m_cInventory.InsertItem ( sSlotItemCustom, wPosX, wPosY );

	// ½½·Ô¾ÆÀÌÅÛ Á¦°Å
	m_sVehicle.ReSetSlotItem ( pNetMsg->emSuit );
	m_fVehicleSpeedRate = m_sVehicle.GetSpeedRate();
	m_fVehicleSpeedVol = m_sVehicle.GetSpeedVol();
	INIT_DATA( FALSE, FALSE );

	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_FB NetMsgFB;
	NetMsgFB.emSuit = pNetMsg->emSuit;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	// ½½·Ô¾ÆÀÌÅÛ Á¦°Å¸¦ ¾Ë¸²
	GLMSG::SNET_VEHICLE_REQ_REMOVE_SLOTITEM_BRD NetMsgBRD;
	NetMsgBRD.dwGaeaID = m_dwGaeaID;
	NetMsgBRD.emSuit = pNetMsg->emSuit;
	SendMsgViewAround ( ( NET_MSG_GENERIC* )&NetMsgBRD );

	return S_OK;
}

HRESULT GLChar::MsgVehicleGiveBattery( NET_MSG_GENERIC* nmg )
{

	GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY* ) nmg;
	
	GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInvenItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	// Å»°Í ¿©ºÎ Ã¼Å©
	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDITEM;
		return E_FAIL;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) 
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	// ¹èÅÍ¸® ¿©ºÎ Ã¼Å©
	if ( pHold->sBasicOp.emItemType != ITEM_VEHICLE_OIL )	
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	// ¾ÆÀÌÅÛ Á¤º¸°¡ ¼­·Î Æ²¸®¸é ½ÇÇàÇÏÁö ¾Ê´Â´Ù.
	SITEM* pPutOnItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if( pHold != pPutOnItem )
	{
		NetMsgFB.emFB = EMVEHICLE_REQ_GIVE_BATTERY_FB_INVALIDBATTERY;
		return E_FAIL;
	}

	sNativeID = pHold->sBasicOp.sNativeID;

	switch ( pHold->sDrugOp.emDrug )
	{
	case ITEM_DRUG_HP:
		{
			if ( pInvenItem->sItemCustom.dwVehicleID == 0 ) return E_FAIL;

			CGetVehicleBattery* pDbAction = new CGetVehicleBattery ( m_dwClientID,
														m_dwCharID,
														pInvenItem->sItemCustom.dwVehicleID,
														pInvenItem->sItemCustom.sNativeID,
														pHold->sDrugOp.wCureVolume, 
														pHold->sDrugOp.bRatio,
														sNativeID,
														pHold->sVehicle.emVehicleType );													   ;

			GLDBMan* pDBMan = GLGaeaServer::GetInstance().GetDBMan ();
			if ( pDBMan ) pDBMan->AddJob ( pDbAction );
		}
		break;

	default:
		break;
	}

	return S_OK;

}

HRESULT	GLChar::MsgGetVehicleFullFromDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_GET_BATTERY_FROMDB_FB* pIntMsg = ( GLMSG::SNET_VEHICLE_REQ_GET_BATTERY_FROMDB_FB* ) nmg;
	switch ( pIntMsg->emFB )
	{
	case EMVEHICLE_REQ_GET_BATTERY_FROMDB_FAIL:
		{
			GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
			NetMsgFB.emFB = EMVEHICLE_REQ_GET_BATTERY_FROMDB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		}
		break;

	case EMVEHICLE_REQ_GET_BATTERY_FROMDB_OK:
		{
			// Æê Æ÷¸¸°¨ ÀÌÀü ·Î±×
			GLITEMLMT::GetInstance().ReqVehicleAction( pIntMsg->dwVehicleID, 
													pIntMsg->sBatteryID,
													EMVEHICLE_ACTION_BATTERY_BEFORE, 
													pIntMsg->nFull );

			int nFull = pIntMsg->nFull;
			int nMaxFull = GLCONST_VEHICLE::pGLVEHICLE[pIntMsg->emType]->m_nFull;
			if ( nFull >= nMaxFull ) break;

			if ( pIntMsg->bRatio )
			{
				nFull += ( nMaxFull*pIntMsg->wCureVolume )/100;
				if ( nFull > nMaxFull ) nFull = nMaxFull;
			}
			else
			{
				nFull += pIntMsg->wCureVolume;
				if ( nFull > nMaxFull ) nFull = nMaxFull;
			}

			CSetVehicleBattery* pDbAction = new CSetVehicleBattery( m_dwClientID, m_dwCharID, pIntMsg->dwVehicleID, nFull );
			GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
			if ( pDbMan ) pDbMan->AddJob ( pDbAction );
			DoDrugSlotItem ( SLOT_HOLD );

			// Æê Æ÷¸¸°¨ ÀÌÀü ·Î±×
			GLITEMLMT::GetInstance().ReqVehicleAction(	pIntMsg->dwVehicleID,
													pIntMsg->sBatteryID,
													EMVEHICLE_ACTION_BATTERY_AFTER,
													nFull );

			// ÇöÀç ·ÎÁ÷¿¡ °¡Áø Å»°Í°ú µ¿ÀÏÇÑ°Å¶ó¸é ·ÎÁ÷À» ¼öÁ¤ÇØÁØ´Ù.
			// Á¤º¸°¡ ºñÈ°¼ºÈ­ µÇ¾îÀÖÁö¸¸ ³ªÁß¿¡ ¹Ù·Î »ç¿ëÇÏ±â À§ÇØ °»½Å
			if ( m_sVehicle.m_dwGUID == pIntMsg->dwVehicleID )	
				m_sVehicle.IncreaseFull ( pIntMsg->wCureVolume, pIntMsg->bRatio );


			GLMSG::SNET_VEHICLE_REQ_GIVE_BATTERY_FB NetMsgFB;
			NetMsgFB.emFB	   = EMVEHICLE_REQ_GIVE_BATTERY_FB_OK;
			NetMsgFB.dwVehicleID = pIntMsg->dwVehicleID;
			NetMsgFB.sItemID	= pIntMsg->sItemID;
			NetMsgFB.sBatteryID = pIntMsg->sBatteryID;
			NetMsgFB.nFull	   = nFull;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
		}
	}

	return S_OK;
}

HRESULT GLChar::MsgSummonSlotExHold( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_SUMMON_REQ_SLOT_EX_HOLD* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_SLOT_EX_HOLD* )nmg;
	GLMSG::SNET_SUMMON_REQ_SLOT_EX_HOLD_FB NetMsgFB;

	PGLSUMMONFIELD pMySummon = m_pGLGaeaServer->GetSummon ( m_dwSummonGUID );
	if( !pMySummon || !pMySummon->IsValid() ) return E_FAIL;

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();
	SITEMCUSTOM	sSlotItemCustom = pMySummon->GetPosionItem();

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );
	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );

	if ( !pHoldItem || !pSlotItem ) 
	{
		// ÀÏ¹Ý¿À·ù
		NetMsgFB.emFB = EMSUMMON_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( !pMySummon->CheckPosionItem( pHoldItem->sBasicOp.sNativeID ) )
	{
		// ¾ÆÀÌÅÛÀÌ È¸º¹Á¦°¡ ¾Æ´Ò¶§
		NetMsgFB.emFB = EMSUMMON_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	// ¼­·Î ´Ù¸¥ Å¸ÀÔÀÏ¶§
	if ( pHoldItem->sSuitOp.emSuit != pSlotItem->sSuitOp.emSuit )
	{
		NetMsgFB.emFB = EMSUMMON_REQ_SLOT_EX_HOLD_FB_NOMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL; 
	}


	// ¼Õ¿¡µç ¾ÆÀÌÅÛÀ» ¼ÒÈ¯¼ö¿¡ ÀåÂø½ÃÅ°°í
	pMySummon->SetPosionItem ( pHoldItem->sBasicOp.sNativeID );
	m_sSummonPosionID = pMySummon->GetPosionItem();

	//	Note : hold ¾ÆÀÌÅÛ ÀÎº¥¿¡ µÇµ¹¸².
	WORD wPosX, wPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pHoldItem->sBasicOp.wInvenSizeX, pHoldItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )		return S_OK;	//	ÀÎº¥ °ø°£ ºÎÁ·À¸·Î µÇµ¹¸² ½ÇÆÐ.

	//	µé°íÀÖ´ø ¾ÆÀÌÅÆ ÀÎ¹ê¿¡ ³ÖÀ½.
	m_cInventory.InsertItem ( GET_HOLD_ITEM(), wPosX, wPosY );

	//	¼Õ¿¡µç ¾ÆÀÌÅÛ Á¦°Å.
	RELEASE_HOLD_ITEM ();

	SINVENITEM *pInvenItem = m_cInventory.GetItem ( wPosX, wPosY );
	if ( !pInvenItem ) return E_FAIL;

	//	[ÀÚ½Å¿¡°Ô] ¸Þ½ÃÁö ¹ß»ý.
	GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
	NetMsg_Inven.Data = *pInvenItem;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);

	//	[ÀÚ½Å¿¡°Ô] ¼Õ ÀÖ¾ú´ø ¾ÆÀÌÅÆ Á¦°Å.
//	GLMSG::SNETPC_PUTON_RELEASE NetMsg_ReleaseHold(SLOT_HOLD);
//	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_ReleaseHold);

	// ÀÚ½Å¿¡°Ô ¾Ë¸²
	NetMsgFB.emFB		 = EMSUMMON_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.sNativeID	 = pMySummon->GetPosionItem();
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	return S_OK;
}

HRESULT GLChar::MsgSummonHoldToSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_SUMMON_REQ_HOLD_TO_SLOT* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_HOLD_TO_SLOT* )nmg;
	GLMSG::SNET_SUMMON_REQ_HOLD_TO_SLOT_FB NetMsgFB;

	PGLSUMMONFIELD pMySummon = m_pGLGaeaServer->GetSummon ( m_dwSummonGUID );
	if( !pMySummon || !pMySummon->IsValid() ) return E_FAIL;

	SITEMCUSTOM sHoldItemCustom = GET_HOLD_ITEM ();

	SITEM* pHoldItem = GLItemMan::GetInstance().GetItem ( sHoldItemCustom.sNativeID );

	if ( !pHoldItem ) 
	{
		// ÀÏ¹Ý¿À·ù
		NetMsgFB.emFB = EMSUMMON_REQ_SLOT_EX_HOLD_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( !pMySummon->CheckPosionItem( pHoldItem->sBasicOp.sNativeID ) )
	{
		// È¸º¹Á¦ ¾ÆÀÌÅÛÀÌ ¾Æ´Ò¶§
		NetMsgFB.emFB = EMSUMMON_REQ_SLOT_EX_HOLD_FB_INVALIDITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	
	// ¼Õ¿¡µç ¾ÆÀÌÅÛÀ» ÆÖ¿¡°Ô ÀåÂø½ÃÅ°°í
	pMySummon->SetPosionItem ( pHoldItem->sBasicOp.sNativeID );
	m_sSummonPosionID = pMySummon->GetPosionItem();


	//	Note : hold ¾ÆÀÌÅÛ ÀÎº¥¿¡ µÇµ¹¸².
	WORD wPosX, wPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pHoldItem->sBasicOp.wInvenSizeX, pHoldItem->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOk )		return S_OK;	//	ÀÎº¥ °ø°£ ºÎÁ·À¸·Î µÇµ¹¸² ½ÇÆÐ.

	//	µé°íÀÖ´ø ¾ÆÀÌÅÆ ÀÎ¹ê¿¡ ³ÖÀ½.
	m_cInventory.InsertItem ( GET_HOLD_ITEM(), wPosX, wPosY );

	//	¼Õ¿¡µç ¾ÆÀÌÅÛ Á¦°Å.
	RELEASE_HOLD_ITEM ();

	SINVENITEM *pInvenItem = m_cInventory.GetItem ( wPosX, wPosY );
	if ( !pInvenItem ) return E_FAIL;

	//	[ÀÚ½Å¿¡°Ô] ¸Þ½ÃÁö ¹ß»ý.
	GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
	NetMsg_Inven.Data = *pInvenItem;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);

	//	[ÀÚ½Å¿¡°Ô] ¼Õ ÀÖ¾ú´ø ¾ÆÀÌÅÆ Á¦°Å.
//	GLMSG::SNETPC_PUTON_RELEASE NetMsg_ReleaseHold(SLOT_HOLD);
//	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_ReleaseHold);

	// ÀÚ½Å¿¡°Ô ¾Ë¸²
	NetMsgFB.emFB	   = EMSUMMON_REQ_SLOT_EX_HOLD_FB_OK;
	NetMsgFB.sPosionID = pMySummon->GetPosionItem();
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	return S_OK;
}


HRESULT GLChar::MsgSummonRemoveSlot( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_SUMMON_REQ_REMOVE_SLOTITEM* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_REMOVE_SLOTITEM* )nmg;

	PGLSUMMONFIELD pMySummon = m_pGLGaeaServer->GetSummon ( m_dwSummonGUID );
	if( !pMySummon || !pMySummon->IsValid() ) return E_FAIL;

	WORD wPosX(0), wPosY(0);
	SITEMCUSTOM	sSlotItemCustom = pMySummon->GetPosionItem();

	SITEM* pSlotItem = GLItemMan::GetInstance().GetItem ( sSlotItemCustom.sNativeID );
	if ( !pSlotItem ) 
	{
		// ÀÏ¹Ý ¿À·ù
		return E_FAIL;
	}

	// ½½·Ô¾ÆÀÌÅÛ Á¦°Å
	pMySummon->ReSetPosionItem();
	m_sSummonPosionID = pMySummon->GetPosionItem();
	INIT_DATA( FALSE, FALSE );

	GLMSG::SNET_SUMMON_REQ_REMOVE_SLOTITEM_FB NetMsgFB;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
	return S_OK;
}


HRESULT GLChar::MsgReqVietnamGainType ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_VIETNAM_TIME_REQ_FB NetMsgFB;
	NetMsgFB.gameTime  = m_sVietnamSystem.gameTime;
	NetMsgFB.loginTime = m_sVietnamSystem.loginTime;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	GLMSG::SNETPC_VIETNAM_GAINTYPE NetMsg;
	NetMsg.dwGainType = (BYTE)m_dwVietnamGainType;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsg );
	return S_OK;
}

HRESULT GLChar::MsgItemShopOpen( NET_MSG_GENERIC* nmg ) // ItemShopOpen
{
#if defined ( _RELEASED)	// JAPAN Item Shop
	
	GLMSG::SNETPC_OPEN_ITEMSHOP* NetMsg = ( GLMSG::SNETPC_OPEN_ITEMSHOP* ) nmg;

	if ( m_bItemShopOpen == NetMsg->bOpen ) return E_FAIL;

	m_bItemShopOpen = NetMsg->bOpen;

	GLMSG::SNETPC_OPEN_ITEMSHOP_BRD NetMsgBrd;
	NetMsgBrd.bOpen = m_bItemShopOpen;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	SendMsgViewAround ( reinterpret_cast<NET_MSG_GENERIC*>(&NetMsgBrd) );

#endif
	
	return S_OK;
}

HRESULT GLChar::MsgAttendReward ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ATTEND_REWARD_FLD* NetMsg = ( GLMSG::SNETPC_REQ_ATTEND_REWARD_FLD* ) nmg;

	GLMSG::SNETPC_REQ_ATTEND_REWARD_CLT	NetMsgFb;

	if ( NetMsg->idAttendReward == NATIVEID_NULL() )
	{
		NetMsgFb.idAttendReward = NetMsg->idAttendReward;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFb );
		return S_OK;
	}

	NetMsgFb.idAttendReward = NetMsg->idAttendReward;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFb );	

	CString strPurKey;
	CString strUserID = m_szUID;
	CTime cCurTime ( CTime::GetCurrentTime() );
	strPurKey.Format( "%04d%02d%02d%s", cCurTime.GetYear(), cCurTime.GetMonth(), cCurTime.GetDay(), strUserID.GetString() );

	CInsertAttendItem* pDbAction = new CInsertAttendItem( strPurKey, strUserID, NetMsg->idAttendReward.wMainID, NetMsg->idAttendReward.wSubID );
	GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDbMan ) pDbMan->AddJob ( pDbAction );

	return S_OK;
}

HRESULT	GLChar::MsgGathering( NET_MSG_GENERIC* nmg )
{
	if ( !m_pLandMan )	return E_FAIL;

	GLMSG::SNETPC_REQ_GATHERING *pNetMsg = reinterpret_cast<GLMSG::SNETPC_REQ_GATHERING*> ( nmg );

	

	PGLMATERIAL pMaterial = m_pLandMan->GetMaterial ( pNetMsg->dwTargetID );

	if ( !pMaterial )
	{
		//	¾ÆÀÌÅÆÀÌ »ç¶óÁ³½À´Ï´Ù.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOCROW;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	if ( pMaterial->GetCrow() != CROW_MATERIAL )
	{
		//	¾ÆÀÌÅÆÀÌ »ç¶óÁ³½À´Ï´Ù.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOTTYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	if ( !pMaterial->IsValidBody()  )
	{
		//	¾ÆÀÌÅÆÀÌ »ç¶óÁ³½À´Ï´Ù.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_NOCROW;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	// »ç¸ÁÈ®ÀÎ
	if ( !IsValidBody() )	return E_FAIL;
	if ( IsACTION( GLAT_GATHERING ) )
	{
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_GATHERING;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	//	°Å¸® Ã¼Å©
	D3DXVECTOR3 vPos;
	vPos = m_vPos;

	const D3DXVECTOR3 &vTarPos = pMaterial->GetPosition();

	D3DXVECTOR3 vDistance = vPos - vTarPos;
	float fDistance = D3DXVec3Length ( &vDistance );

	WORD wTarBodyRadius = 4;
	WORD wGatherRange = wTarBodyRadius + GETBODYRADIUS() + 2;
	WORD wGatherAbleDis = wGatherRange + 15;

	if ( fDistance>wGatherAbleDis )
	{
		//	°Å¸®°¡ ¸Ù´Ï´Ù.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_DISTANCE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

		return E_FAIL;
	}


	// ÀÌ¹Ì ´Ù¸¥»ç¶÷ÀÌ »ç¿ëÁßÀÌ¶ó¸é???

	if ( pMaterial->m_dwGatherCharID != GAEAID_NULL )
	{
		//	°Å¸®°¡ ¸Ù´Ï´Ù.
		GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
		NetMsg.emFB = EMREQ_GATHER_FB_USE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );
		return E_FAIL;
	}

	int nTime = RandomNumber( pMaterial->m_pCrowData->m_wGatherTimeLow, pMaterial->m_pCrowData->m_wGatherTimeHigh );
	// crow¿¡ ¼ÂÆÃ
	pMaterial->m_dwGatherCharID = m_dwGaeaID;
	pMaterial->m_fGatherTime = (float)nTime;

	// random Å¸ÀÔ ±¸ÇöÈÄ¿¡ ½Ã°£ Ã¼Å©

	//	User¿¡ ¼ÂÆÃ
	TurnAction ( GLAT_GATHERING );

	m_dwANISUBSELECT = pMaterial->m_pCrowData->m_wGatherAnimation;

	//	°Å¸®°¡ ¸Ù´Ï´Ù.
	GLMSG::SNETPC_REQ_GATHERING_FB NetMsg;
	NetMsg.emFB = EMREQ_GATHER_FB_OK;
	NetMsg.dwTime = nTime; // pCrow->dwTime;
	NetMsg.dwAniSel = m_dwANISUBSELECT;
	NetMsg.dwGaeaID = pMaterial->m_dwGlobID;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsg );

	GLMSG::SNETPC_REQ_GATHERING_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwAniSel = m_dwANISUBSELECT;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

HRESULT GLChar::MsgProcess ( NET_MSG_GENERIC* nmg )
{
	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_REQ_READY:			MsgReady(nmg);					break;
	case NET_MSG_GCTRL_ACTSTATE:			MsgActState(nmg);				break;
	case NET_MSG_GCTRL_MOVESTATE:			MsgMoveState(nmg);				break;

	case NET_MSG_GCTRL_GOTO:				MsgGoto(nmg);					break;

	case NET_MSG_GCTRL_ATTACK:				MsgAttack(nmg);					break;

	case NET_MSG_GCTRL_ATTACK_CANCEL:		MsgAttackCancel(nmg);			break;

	case NET_MSG_GCTRL_REQ_FIELD_TO_INVEN:	MsgReqFieldToInven(nmg);		break;
	case NET_MSG_GCTRL_REQ_FIELD_TO_HOLD:	MsgReqFieldToHold(nmg);			break;

	case NET_MSG_GCTRL_REQ_INVEN_TO_HOLD:	MsgReqInvenToHold(nmg); 		break;
	case NET_MSG_GCTRL_REQ_INVEN_EX_HOLD:	MsgReqInvenExHold(nmg); 		break;
	case NET_MSG_GCTRL_REQ_INVEN_TO_SLOT:	MsgReqInvenToSlot(nmg);			break;

	case NET_MSG_GCTRL_REQ_VNGAIN_TO_HOLD:  MsgReqVNGainToHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_VNGAIN_EX_HOLD:  MsgReqVNGainExHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_VNGAIN:	MsgReqHoldToVNGain(nmg);		break;
	case NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN: MsgReqVNInvenToInven(nmg);		break;

	case NET_MSG_GCTRL_REQ_VNGAIN_INVEN_RESET: MsgReqVNGainInvenReset(nmg);		break;

	case NET_MSG_GCTRL_REQ_SLOT_TO_HOLD:	MsgReqSlotToHold(nmg);			break;
	case NET_MSG_GCTRL_REQ_SLOT_EX_HOLD:	MsgReqSlotExHold(nmg);			break;


	case NET_MSG_GCTRL_REQ_INVEN_SPLIT:		MsgReqInvenSplit(nmg);			break;


	case NET_MSG_GCTRL_REQ_HOLD_TO_FIELD:	MsgReqHoldToField(nmg); 		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_INVEN:	MsgReqHoldToInven(nmg); 		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_SLOT:	MsgReqHoldToSlot(nmg);			break;
	case NET_MSG_GCTRL_REQ_SLOT_CHANGE:		MsgReqSlotChange(nmg);			break;

	case NET_MSG_GCTRL_REQ_BUY_FROM_NPC:	MsgReqBuyFromNpc(nmg);			break;
	case NET_MSG_GCTRL_REQ_SALE_TO_NPC:		MsgReqSaleToNpc(nmg);			break;

	case NET_MSG_GCTRL_REQ_REBIRTH:			MsgReqReBirth(nmg);				break;
	case NET_MSG_GCTRL_REQ_LEVELUP:			MsgReqLevelUp(nmg);				break;
	case NET_MSG_GCTRL_REQ_STATSUP:			MsgReqStatsUp(nmg);				break;

	case NET_MSG_GCTRL_REQ_STATSUPCMD:		MsgReqStatsUpCmd(nmg);			break; // add addstats cmd

	case NET_MSG_GCTRL_REQ_LEARNSKILL:		MsgReqLearnSkill(nmg);			break;
	case NET_MSG_GCTRL_REQ_SKILLUP:			MsgReqSkillUp(nmg);				break;

	case NET_MSG_GCTRL_FIRECRACKER:			MsgReqFireCracker(nmg);			break;
	case NET_MSG_GCTRL_REQ_INVENDRUG:		MsgReqInvenDrug(nmg);			break;
	case NET_MSG_GCTRL_INVEN_BOXOPEN:		MsgReqInvenBoxOpen(nmg);		break;
	case NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB: MsgReqInvenBoxInfo(nmg);		break;
	case NET_MSG_GCTRL_INVEN_DISGUISE:		MsgReqInvenDisguise(nmg);		break;
	case NET_MSG_GCTRL_INVEN_CLEANSER:		MsgReqInvenCleanser(nmg);		break;
	case NET_MSG_GCTRL_CHARGED_ITEM_GET:	MsgReqInvenChargedItem(nmg);	break;
	case NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN:	MsgReqInvenRandomBoxOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_DISJUNCTION:	MsgReqInvenDisJunction(nmg);	break;

	case NET_MSG_GCTRL_REQ_SKILL:			MsgReqSkill(nmg);				break;
	case NET_MSG_GCTRL_SKILL_CANCEL:		MsgReqSkillCancel(nmg);			break;

	case NET_MSG_GCTRL_REQ_SKILLQ_ACTIVE:	MsgReqSkillQSetActive(nmg);		break;
	case NET_MSG_GCTRL_REQ_SKILLQ_SET:		MsgReqSkillQSet(nmg);			break;
	case NET_MSG_GCTRL_REQ_SKILLQ_RESET:	MsgReqSkillQReSet(nmg);			break;

	case NET_MSG_GCTRL_REQ_ACTIONQ_SET:		MsgReqActionQSet(nmg);			break;
	case NET_MSG_GCTRL_REQ_ACTIONQ_RESET:	MsgReqActionQReSet(nmg);		break;

	case NET_MSG_GCTRL_REQ_GETSTORAGE:		MsgReqGetStorage(nmg);			break;
	case NET_MSG_GCTRL_REQ_STORAGEDRUG:		MsgReqStorageDrug(nmg);			break;
	case NET_MSG_GCTRL_REQ_STORAGESKILL:	MsgReqStorageSkill(nmg);		break;

	case NET_MSG_GCTRL_REQ_STORAGE_TO_HOLD:	MsgReqStorageToHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_STORAGE_EX_HOLD:	MsgReqStorageExHold(nmg);		break;
	case NET_MSG_GCTRL_REQ_HOLD_TO_STORAGE:	MsgReqHoldToStorage(nmg);		break;

	case NET_MSG_GCTRL_STORAGE_SAVE_MONEY:	MsgReqStorageSaveMoney(nmg);	break;
	case NET_MSG_GCTRL_STORAGE_DRAW_MONEY:	MsgReqStorageDrawMoney(nmg);	break;


	case NET_MSG_GCTRL_REQ_STORAGE_SPLIT:	MsgReqStorageSplit(nmg);		break;


	case NET_MSG_GCTRL_REQ_MONEY_TO_FIELD:	MsgReqMoneyToField(nmg);		break;
	case NET_MSG_GCTRL_INVEN_GRINDING:		MsgReqInvenGrinding(nmg);		break;
	case NET_MSG_GCTRL_INVEN_RESET_SKST:	MsgReqInvenResetSkSt(nmg);		break;

	case NET_MSG_GCTRL_REGEN_GATE:			MsgReqReGenGate(nmg);			break;
	case NET_MSG_GCTRL_CURE:				MsgReqCure(nmg);				break;
	case NET_MSG_GCTRL_REVIVE:				MsgReqRevive(nmg);				break;
	case NET_MSG_GCTRL_2_FRIEND_FLD:		MsgReqFriendFld(nmg);			break;
	case NET_MSG_GM_MOVE2CHAR_FLD:			MsgReqMove2CharFld(nmg);		break;

	case NET_MSG_GCTRL_GETEXP_RECOVERY:		MsgReqGetExpRecovery(nmg);		break;
	case NET_MSG_GCTRL_RECOVERY:			MsgReqRecovery(nmg);			break;
	case NET_MSG_GCTRL_GETEXP_RECOVERY_NPC:	MsgReqGetExpRecoveryNpc(nmg);	break;
	case NET_MSG_GCTRL_RECOVERY_NPC:		MsgReqRecoveryNpc(nmg);			break;

	case NET_MSG_GCTRL_CHARRESET:			MsgReqCharReset(nmg);			break;
	case NET_MSG_GCTRL_INVEN_CHARCARD:		MsgReqCharCard(nmg);			break;
	case NET_MSG_GCTRL_INVEN_STORAGECARD:	MsgReqStorageCard(nmg);			break;
	case NET_MSG_GCTRL_INVEN_INVENLINE:		MsgReqInvenLine(nmg);			break;
	case NET_MSG_GCTRL_INVEN_STORAGEOPEN:	MsgReqInvenStorageOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_REMODELOPEN:	MsgReqInvenRemodelOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_GARBAGEOPEN:	MsgReqInvenGarbageOpen(nmg);	break;
	case NET_MSG_GCTRL_INVEN_STORAGECLOSE:	MsgReqInvenStorageClose(nmg);	break;
	case NET_MSG_GCTRL_INVEN_PREMIUMSET:	MsgReqInvenPremiumSet(nmg);		break;

	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE:	MsgReqInvenHairChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_FACE_CHANGE:	MsgReqInvenFaceChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_HAIRSTYLE_CHANGE: MsgReqInvenHairStyleChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE: MsgReqInvenHairColorChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_FACESTYLE_CHANGE: MsgReqInvenFaceStyleChange(nmg);		break;
	case NET_MSG_GCTRL_INVEN_GENDER_CHANGE:	MsgReqInvenGenderChange(nmg);	break;
	case NET_MSG_GCTRL_INVEN_RENAME:		MsgReqInvenRename(nmg);			break;
	case NET_MSG_GCTRL_INVEN_RENAME_FROM_DB: MsgInvenRename(nmg);			break;

	case NET_MSG_GCTRL_INVEN_VIETNAM_INVENGET: MsgInvenVietnamGet(nmg);		break;

	case NET_MSG_GCTRL_CONFRONT_START2_FLD:	MsgReqConFrontStart(nmg);		break;
	case NET_MSG_GCTRL_CONFRONT_END2_FLD:	MsgReqConFrontEnd(nmg);			break;

	case NET_MSG_GCTRL_NPC_ITEM_TRADE:		MsgReqNpcItemTrade(nmg);		break;

	case NET_MSG_GCTRL_REQ_QUEST_START:		MsgReqNpcQuestStart(nmg);		break;
	case NET_MSG_GCTRL_QUEST_PROG_NPCTALK:	MsgReqNpcQuestTalk(nmg);		break;

	case NET_MSG_GCTRL_QUEST_PROG_GIVEUP:	MsgReqQuestGiveUp(nmg);			break;
	case NET_MSG_GCTRL_QUEST_PROG_READ:		MsgReqQuestReadReset(nmg);		break;
	case NET_MSG_GCTRL_REQ_QUEST_COMPLETE:	MsgReqQuestComplete(nmg);		break;

	case NET_MSG_GCTRL_REQ_GESTURE:			MsgReqGesture ( nmg );			break;

	case NET_MSG_CHAT_LOUDSPEAKER:			MsgLoudSpeaker(nmg);			break;

	case NET_MSG_GCTRL_PMARKET_TITLE:		MsgPMarketTitle(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_REGITEM:		MsgPMarketReqItem(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_DISITEM:		MsgPMarketDisItem(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_OPEN:		MsgPMarketOpen(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_CLOSE:		MsgPMarketClose(nmg);			break;
	case NET_MSG_GCTRL_PMARKET_ITEM_INFO:	MsgPMarketItemInfo(nmg);		break;
	case NET_MSG_GCTRL_PMARKET_BUY:			MsgPMarketBuy(nmg);				break;

	case NET_MSG_GCTRL_CLUB_NEW:			MsgClubNew(nmg);				break;
	case NET_MSG_GCTRL_CLUB_RANK:			MsgClubRank(nmg);				break;
	case NET_MSG_GCTRL_CLUB_NEW_2FLD:		MsgClubNew2FLD(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ:		MsgClubMemberReq(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ_ANS:	MsgClubMemberAns(nmg);			break;
	case NET_MSG_GCTRL_CLUB_MEMBER_NICK:	MsgClubMemberNick(nmg);			break;

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY:		MsgClubCDCertify(nmg);			break;
	case NET_MSG_GCTRL_CLUB_GETSTORAGE:		MsgReqClubGetStorage(nmg);		break;

	case NET_MSG_GCTRL_CLUB_STORAGE_TO_HOLD:	MsgReqClubStorageToHold(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_EX_HOLD:	MsgReqClubStorageExHold(nmg);	break;
	case NET_MSG_GCTRL_CLUB_HOLD_TO_STORAGE:	MsgReqClubHoldToStorage(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_SPLIT:		MsgReqClubStorageSplit(nmg);	break;
	case NET_MSG_GCTRL_CLUB_STORAGE_SAVE_MONEY:	MsgReqClubStorageSaveMoney(nmg);break;
	case NET_MSG_GCTRL_CLUB_STORAGE_DRAW_MONEY:	MsgReqClubStorageDrawMoney(nmg);break;

	case NET_MSG_GCTRL_CLUB_INCOME_RENEW:	MsgReqClubInComeReNew(nmg);		break;

	case NET_MSG_REBUILD_RESULT:			MsgReqRebuildResult(nmg);		break;	// ITEMREBUILD_MARK
	case NET_MSG_REBUILD_MOVE_ITEM:			MsgReqRebuildMoveItem(nmg);		break;
	case NET_MSG_REBUILD_INPUT_MONEY:		MsgReqRebuildInputMoney(nmg);	break;

	case NET_MSG_GCTRL_GARBAGE_RESULT:		MsgReqGarbageResult(nmg);		break;

	case NET_MSG_SMS_PHONE_NUMBER:			MsgReqPhoneNumber(nmg);			break;
	case NET_MSG_SMS_PHONE_NUMBER_FROM_DB:	MsgPhoneNumber(nmg);			break;
	case NET_MSG_SMS_SEND:					MsgReqSendSMS(nmg);				break;
	case NET_MSG_SMS_SEND_FROM_DB:			MsgSendSMS(nmg);				break;

	case NET_MSG_MGAME_ODDEVEN:				MsgReqMGameOddEvenEvent(nmg);	break; // ¹Ì´Ï°ÔÀÓ - È¦Â¦

	case NET_MSG_GCTRL_ITEMSHOPOPEN:		MsgItemShopOpen ( nmg );		break;
	
	case NET_MSG_GCTRL_INVEN_ITEM_MIX:		MsgReqItemMix( nmg );			break;

		// PET
	case NET_MSG_PET_GETRIGHTOFITEM:		MsgReqGetRightOfItem ( nmg );	break;
	case NET_MSG_PET_REQ_GIVEFOOD:			MsgGiveFood ( nmg );			break;
	case NET_MSG_PET_REQ_GETFULL_FROMDB_FB: MsgGetPetFullFromDB ( nmg );    break;
	case NET_MSG_PET_REQ_PETREVIVEINFO:     MsgPetReviveInfo ( nmg );       break;

	// vehicle
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD:	MsgVehicleSlotExHold ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_HOLD_TO_SLOT:	MsgVehicleHoldToSlot ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_SLOT_TO_HOLD:	MsgVehicleSlotToHold ( nmg );	break;
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM:	MsgVehicleRemoveSlot ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_GIVE_BATTERY:	MsgVehicleGiveBattery ( nmg );	break;
	case NET_MSG_VEHICLE_REQ_GET_BATTERY_FROMDB_FB:	MsgGetVehicleFullFromDB ( nmg );	break;

	// Summon
	case NET_MSG_SUMMON_REQ_SLOT_EX_HOLD:	MsgSummonSlotExHold ( nmg );	break;
	case NET_MSG_SUMMON_REQ_HOLD_TO_SLOT:	MsgSummonHoldToSlot ( nmg );	break;
	case NET_MSG_SUMMON_REMOVE_SLOTITEM:	MsgSummonRemoveSlot ( nmg );	break;

	// vietNam GainType System
	case NET_MSG_VIETNAM_TIME_REQ:				MsgReqVietnamGainType ( nmg );  break;

		// ETC
	case NET_MSG_GCTRL_SET_SERVER_DELAYTIME:	MsgSetServerDelayTime ( nmg ); break;
	case NET_MSG_GCTRL_SET_SERVER_SKIPPACKET:	MsgSetSkipPacketNum ( nmg );  break;
	case NET_MSG_GCTRL_SET_SERVER_CRASHTIME:	MsgSetServerCrashTime ( nmg ); break;

	case NET_MSG_REQ_ATTEND_REWARD_FLD:			MsgAttendReward( nmg );	break;
	
	case NET_MSG_REQ_GATHERING:					MsgGathering(nmg);	break;
	case NET_MSG_REQ_GATHERING_CANCEL:			MsgGatheringCancel(nmg);			break;


	default:
		{			
			DEBUGMSG_WRITE(_T("[WARNING] GLCharMsg::MsgProcess illigal message (%d)"),
				nmg->nType );
		}
		break;
	}

	return S_OK;
}
