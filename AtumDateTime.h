#pragma once
//Copyright [2002] MasangSoft
#ifdef _ATUM_SERVER
#include <SQLTYPES.H>
#endif

#include <WINDOWS.H>
#include <TIME.H>

//#include <string>
//
//
//#if defined(_ATUM_SERVER)
//#include "StringDefineQuery.h"            // 2008-07-30 by cmkwon
//#include "StringDefineProcedure.h"    // 2008-08-22 by cmkwon, MySQL ����(Procedure �̾Ƴ���) - 
//#endif

using namespace std;

// 2008-01-04 by cmkwon, ���� ������� �Ϸ��� �Ʒ��� ���� ���� �̾�� ��
//#define SQL_DATETIME_STRING_FORMAT                    "%04d-%02d-%02d %02d:%02d:%02d.000"

///////////////////////////////////////////////////////////////////////////////
// 2008-07-08 by cmkwon, MySQL ���� ���� - 
#ifdef DB_SERVER_MYSQL
#define SQL_DATETIME_STRING_FORMAT                    "%04d-%02d-%02d %02d:%02d:%02d.000"
#else
#define SQL_DATETIME_STRING_FORMAT                    "%04d%02d%02d %02d:%02d:%02d.000"
#endif

#define ATUM_DATE_TIME_STRING_FORMAT                "%04d-%02d-%02d %02d:%02d:%02d"
#define ATUM_DATE_TIME_STRING_FORMAT_EXCLUDE_SECOND    "%04d-%02d-%02d %02d:%02d"

#define FILE_DATETIME_STRING_FORMAT                    "%04d%02d%02d_%02d%02d%02d"        // 2013-05-28 by hskim, �̴� ���� ���� �̸��� �ð� �߰�

#define SIZE_MAX_SQL_DATETIME_STRING    30
#define SIZE_MAX_ATUM_DATE_TIME_STRING    25

#define GET_CURRENT_DATE_TIME_STRING()    ATUM_DATE_TIME::GetCurrentDateTimeString().GetBuffer()

struct STRNBUF;

struct ATUM_DATE_TIME
{
    static_assert(sizeof(time_t) == 8, "The struct ATUM_DATE_TIME may be affected by the Year 2038 problem.");

    USHORT        Year;
    BYTE        Month;
    BYTE        Day;
    BYTE        Hour;
    BYTE        Minute;
    BYTE        Second;

    ATUM_DATE_TIME() : Year { 0 }, Month { 0 }, Day { 0 }, Hour { 0 }, Minute { 0 }, Second { 0 } { CheckSettm_isdst(); }
    explicit ATUM_DATE_TIME(bool i_bSetCurrentTime) : ATUM_DATE_TIME { } { if (i_bSetCurrentTime) SetCurrentDateTime(); }
    explicit ATUM_DATE_TIME(const char *i_szDateTimeString) : ATUM_DATE_TIME { } { SetDateTime(i_szDateTimeString); }
    explicit ATUM_DATE_TIME(time_t rvalue);

    void Reset() { Year = Month = Day = Hour = Minute = Second = 0; }

    void SetCurrentDateTime(BOOL i_bSetSecondZero = FALSE);
    void SetDateTime(const char *i_szDateTimeString);
    void SetDateTime(USHORT i_usYear, BYTE i_byMonth, BYTE i_byDay, BYTE i_byHour, BYTE i_byMinute, BYTE i_bySecond);

    void AddDateTime(INT i_nYear, INT i_nMonth, INT i_nDay, INT i_nHour = 0, INT i_nMinute = 0, INT i_nSecond = 0);

    const char* GetDateTimeString(char *i_szBuffer, int i_nLen, BOOL i_bExcludeSecond = FALSE) const;
    const char* GetSQLDateTimeString(char *i_szBuffer, int i_nLen) const;
    //const char* GetDateTimeString(const STRNBUF &i_strnbuf, BOOL i_bExcludeSecond = FALSE) const;
    STRNBUF GetDateTimeString(bool i_bExcludeSecond = false) const;
    const char* GetSQLDateTimeString(STRNBUF &i_strnbuf) const;
    const char* GetFileDateTimeString(STRNBUF &i_strnbuf) const;

    int GetTimeInSeconds() const;
    int GetTimeDiffToCurrentTimeInSeconds() const;
    int GetTimeDiffTimeInSeconds(ATUM_DATE_TIME i_atOP2) const;
    int GetTimeDiffTimeInSeconds(int i_atOP2) const;
    int GetTimeDiffTimeInMinutes(ATUM_DATE_TIME i_atOP2) const;
    int GetTimeDiffTimeInHours(ATUM_DATE_TIME i_atOP2) const;

    bool IsValid() const { return Year != 0 && Month != 0 && Day != 0; }

    const char* GetLocalDateTimeString(string &o_szStrBuf, BOOL i_bExcludeSecond = FALSE) const;        // 2008-03-18 by cmkwon, ���� ��¥ ���� ���� - 

    // Static members

    static ATUM_DATE_TIME GetCurrentDateTime();
    static STRNBUF GetCurrentDateTimeString();
    static const char* GetDateTimeStringFromSeconds(int i_nSeconds, string &i_szStrBuf);
    static const char* GetLocalString_YYYYMMDD(int i_Year, int i_Month, int i_Day, string &io_szStrBuf);    // 2007-10-05 by cmkwon, �������� ���󺰷� �ٸ����ϱ� ����
    static const char* GetLocalString_YYYYMM(int i_Year, int i_Month, string &io_szStrBuf);                    // 2007-10-05 by cmkwon, �������� ���󺰷� �ٸ����ϱ� ����
    static const char* GetLocalString_MMDD(int i_Month, int i_Day, string &io_szStrBuf);                    // 2007-10-05 by cmkwon, �������� ���󺰷� �ٸ����ϱ� ����
    static void CheckSettm_isdst();

    // 2008-04-02 by cmkwon, ���Ÿ�� ���� ���� ���� - 
    static bool ms_bSettm_isdst;        // �Ʒ��� ���Ÿ�� ���� ���� ���� �ߴ��� ���� �÷���
    static int ms_tm_isdst;            // ���Ÿ�� ���� ���� -  0�̸� ǥ�ؽø� ����ϴ� ���̰�, ������ ���� ���� ���̰�, ������� TZ(time zone) ȯ�� ������ ���� ǥ�ؽø� ����ϰų� �ϱ� ���� �ð��� ����մϴ�

#ifdef _ATUM_SERVER
    ATUM_DATE_TIME& operator=(const SQL_TIMESTAMP_STRUCT& rhs);
    void Convert(SQL_TIMESTAMP_STRUCT& o_SQL_TIMESTAMP_STRUCT) const;
#endif

    bool operator==(const ATUM_DATE_TIME& dt) const
    {
        if (Year == dt.Year && Month == dt.Month && Day == dt.Day && Hour == dt.Hour && Minute == dt.Minute && Second == dt.Second) return true;

        return false;
    }
    bool operator!=(const ATUM_DATE_TIME& dt) const
    {
        if (Year == dt.Year && Month == dt.Month && Day == dt.Day && Hour == dt.Hour && Minute == dt.Minute && Second == dt.Second) return false;

        return true;
    }
    bool operator<(const ATUM_DATE_TIME& dt) const
    {
        if (Year != dt.Year) return Year < dt.Year;
        if (Month != dt.Month) return Month < dt.Month;
        if (Day != dt.Day) return Day < dt.Day;
        if (Hour != dt.Hour) return Hour < dt.Hour;
        if (Minute != dt.Minute) return Minute < dt.Minute;
        if (Second != dt.Second) return Second < dt.Second;

        return false;
    }
    bool operator>(const ATUM_DATE_TIME& dt) const
    {
        if (Year != dt.Year) return Year > dt.Year;
        if (Month != dt.Month) return Month > dt.Month;
        if (Day != dt.Day) return Day > dt.Day;
        if (Hour != dt.Hour) return Hour > dt.Hour;
        if (Minute != dt.Minute) return Minute > dt.Minute;
        if (Second != dt.Second) return Second > dt.Second;

        return false;
    }
    bool operator<=(const ATUM_DATE_TIME& dt) const
    {
        if (Year != dt.Year) return Year < dt.Year;
        if (Month != dt.Month) return Month < dt.Month;
        if (Day != dt.Day) return Day < dt.Day;
        if (Hour != dt.Hour) return Hour < dt.Hour;
        if (Minute != dt.Minute) return Minute < dt.Minute;
        if (Second != dt.Second) return Second < dt.Second;

        return true;
    }
    bool operator>=(const ATUM_DATE_TIME& dt) const
    {
        if (Year != dt.Year) return Year > dt.Year;
        if (Month != dt.Month) return Month > dt.Month;
        if (Day != dt.Day) return Day > dt.Day;
        if (Hour != dt.Hour) return Hour > dt.Hour;
        if (Minute != dt.Minute) return Minute > dt.Minute;
        if (Second != dt.Second) return Second > dt.Second;

        return true;
    }

    explicit operator tm() const;
};
