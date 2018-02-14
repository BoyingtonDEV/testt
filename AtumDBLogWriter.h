//Copyright [2002] MasangSoft
#ifndef _ATUM_DB_LOG_WRITER_H_
#define _ATUM_DB_LOG_WRITER_H_

class CODBCStatement;

extern const char* GetLogBaseString(FL_USER_LOG_BASE *pLogBase, string& str);

class CAtumDBLogWriter  
{
public:
	CAtumDBLogWriter();
	virtual ~CAtumDBLogWriter();
// 2006-09-14 by cmkwon, ������� �ʴ� �Լ���
// 	static BOOL InsertLog_Connection(
// 							CODBCStatement	*i_pODBCStmt,
// 							BYTE			i_nLogType,
// 							BYTE			*i_arrIPAddr,
// 							UID32_t			i_nAccountUniqueNumber,
// 							char			*i_szAccountName,
// 							char			*i_szServerName);
// 
// 	static BOOL InsertLog_User_Level_Stat(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_USER_LOG_BASE	*i_pUserLogBase,
// 							USHORT				i_nParam1,
// 							USHORT				i_nParam2,
// 							LONGLONG			i_nParam3);
// 
// 	static BOOL InsertLog_User_Exp(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_USER_LOG_BASE	*i_pUserLogBase,
// 							FLOAT				i_fParam1,
// 							LONGLONG			i_nParam2);
// 
	
	// 2008-01-17 by cmkwon, T_A: ���� ������ ĳ���� ������ �����ϴ� ��� �߰� - CAtumDBLogWriter::InsertLog_User_Game_Start_End() �� �ּ����¸� ���� ��Ŵ
	static BOOL InsertLog_User_Game_Start_End(
							CODBCStatement		*i_pODBCStmt,
							T1<T0::FL_LOG>		i_nLogType,
							FL_USER_LOG_BASE	*i_pUserLogBase,
							char				*i_szCharacterName,
							UID32_t				i_nAccountUniqueNumber,
							char				*i_szAccountName,
							UINT				i_nPlayTime,
							LONGLONG			I_nTotalPlayTime);

// 
// 	static BOOL InsertLog_User_Quest_PK_Dead(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_USER_LOG_BASE	*i_pUserLogBase,
// 							INT					i_nParam1);
// 
// 	static BOOL InsertLog_User_WARP(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_USER_LOG_BASE	*i_pUserLogBase,
// 							MapIndex_t			i_nTargetMapIndex,
// 							AVECTOR3			&i_TargetPosition);
// 
// 	static BOOL InsertLog_Item_Charac_Use(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_ITEM_LOG_BASE	*i_pItemLogBase,
// 							ITEM_FOR_LOG		*i_pItem4Log,
// 							int					i_nLogParam1=-1);
// 
// 	static BOOL InsertLog_Item_Get_Throw(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_ITEM_LOG_BASE	*i_pItemLogBase,
// 							ITEM_FOR_LOG		*i_pItem4Log,
// 							INT					i_nChangeCount);
// 
// 	static BOOL InsertLog_Item_Buy_Sell(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							FL_ITEM_LOG_BASE	*i_pItemLogBase,
// 							ITEM_FOR_LOG		*i_pItem4Log,
// 							INT					i_nChangeCount,
// 							INT					i_nRemainedMoney);
// 


	// 2012-11-21 by bckim, �������߰�, ��æƮ���������ű��
	static BOOL CAtumDBLogWriter::InsertLog_Item_Enchant_Change_By_Admin(
							CODBCStatement		*i_pODBCStmt,
							T1<T0::FL_LOG>		i_nLogType,
							FL_ITEM_LOG_BASE	*i_pItemLogBase,		// �ɸ��� ����ũ �ѹ� 
							ITEM_FOR_LOG		*i_pItem4Log,			// ItemUniqueNumber;  ItemNum; CurrentCount;
							INT					i_EnchantCardNumber,	// ��æƮ �����۳ѹ�
							INT					i_ChangeEnchantCount,	// ������ ��æƮ ī��Ʈ 
							INT					i_ResultEnchantCount,	// ��� ��æƮ ī��Ʈ	
							char				*i_szCharacterName);		// ��� ����

	// 2012-11-21 by bckim, �������߰�, ��æƮ���������ű��


	// 2006-09-21 by cmkwon, SCAdminTool���� ����ϰ� ����
 	static BOOL InsertLog_Item_Trade(
 							CODBCStatement		*i_pODBCStmt,
							T1<T0::FL_LOG>		i_nLogType,
 							FL_ITEM_LOG_BASE	*i_pItemLogBase,
 							ITEM_FOR_LOG		*i_pItem4Log,
 							INT					i_nChangeCount,
 							char				*i_szCharacterName,
 							UID32_t				i_nCharacterUniqueNumber);
 
// 	static BOOL InsertLog_Server_Map(
// 							CODBCStatement		*i_pODBCStmt,
// 							BYTE				i_nLogType,
// 							MapIndex_t			i_nMapIndex,
// 							ChannelIndex_t		i_nChannelIndex,
// 							INT					i_nClientCount,
// 							INT					i_nMonsterCount);
// 
// 	static BOOL InsertLog_Speed_Hack(CODBCStatement	*i_pODBCStmt, MSG_FL_LOG_SPEED_HACK *i_pMsgSpeedHack);
// 	static BOOL InsertLog_Memory_Hack(CODBCStatement *i_pODBCStmt, MSG_FL_LOG_MEMORY_HACK *i_pMsgMemoryHack);
// 
// 	static BOOL InsertLog_Monster_Boss(
// 							CODBCStatement			*i_pODBCStmt,
// 							MSG_FL_LOG_MONSTER_BOSS	*i_pMsgMonsterBoss,
// 							const char				*i_szDropItemListString);
// 
// 	static BOOL MGame_PB_Init(CODBCStatement			*i_pODBCStmt);
	
	static BOOL InserLog_Live_Deleted_Character(
							CODBCStatement			*i_pODBCStmt,
							T1<T0::FL_LOG>		i_nLogType,
							SDELETED_CHARACTER_INFO	*i_pDeletedCharacterInfo);	// 2007-02-22 by dhjin, �ɸ��� ���� �α� �����.
};

#endif // _ATUM_DB_LOG_WRITER_H_
