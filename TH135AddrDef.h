#ifndef TH135ADDRDEF_H_INCLUDED
#define TH135ADDRDEF_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define IsInParamList(p)	p-TH135PARAM_MAX>=0
#define PL2P(p) p+TH135PARAM_MAX
#define P2PL(p) p-TH135PARAM_MAX



typedef struct {
	LPCSTR target;
	DWORD vtype;
	PCTSTR outstr;
	
}TH135GETPARAM;


extern const int s_paramcnt;
extern int s_paramOld[];



enum TH135CBMSG {
	TH135MSG_STATECHANGE,
	TH135MSG_PARAMCHANGE,
};

enum TH135STATE {
	TH135STATE_NOTFOUND,
	TH135STATE_WAITFORNETBATTLE,
	TH135STATE_NETBATTLE
};

enum TH135SCHAR {
	TH135CHAR_REIMU = 0,
	TH135CHAR_MARISA = 1,
	TH135CHAR_ICHIRIN = 2,
	TH135CHAR_LIMIT = 10,
};

struct TH135COMBOSAVE{
	time_t timestamp;
	time_t Battle;
	char pname[MAX_PATH];
	int damage;
	int stun;
	int rate;
	int hit;
};

enum TH135PARAM {
	TH135PARAM_BATTLESTATE,
	TH135PARAM_ISNETCLIENT,
	TH135PARAM_P1CHAR,
	TH135PARAM_P2CHAR,
	TH135PARAM_P1WIN,
	TH135PARAM_P2WIN,
	TH135PARAM_TOADDR,
	TH135PARAM_P1NAME,
	TH135PARAM_P2NAME,
	/*
	TH135PARAM_P1HPB,
	TH135PARAM_P1HPF,
	TH135PARAM_P2HPB,
	TH135PARAM_P2HPF,
	TH135PARAM_P1GOOD,
	TH135PARAM_P1HATE,
	TH135PARAM_P1DAM,
	TH135PARAM_P1STUN,
	TH135PARAM_P1RATE,
	TH135PARAM_P1HIT,
	TH135PARAM_P2GOOD,
	TH135PARAM_P2HATE,
	TH135PARAM_P2DAM,
	TH135PARAM_P2STUN,
	TH135PARAM_P2RATE,
	TH135PARAM_P2HIT,
	*/
	TH135PARAM_LISTCHANGED,

	TH135PARAM_MAX,
};

typedef void (__cdecl *TH135ADDRCALLBACKPROC)(int, int, int, void *);

typedef struct {
	PCTSTR full;
	PCTSTR abbr;
} TH135CHARNAME;



int TH135AddrInit(HWND, int);
int TH135AddrFinish();
DWORD_PTR TH135AddrGetParam(int);
TH135STATE TH135AddrGetState();
const TH135CHARNAME * const TH135AddrGetCharName(int index);
int TH135AddrGetCharCount();

inline double GetLifeReduce(int life)
{
	return life>=5000 ? 1 : (75+life*0.0025)/100;
}

inline double GetLifeReduceDamage(int life,int damage)
{
	return life>=5000 ? damage : damage*(75+life*0.0025)/100;
}

inline double GetOriginDamageLR(int life,double damage)
{
	return life>=5000 ? damage : damage*100/(75+life*0.0025);
}


#ifdef __cplusplus
}
#endif

extern TH135GETPARAM s_paramlist[];

#endif /* TH135ADDRDEF_H_INCLUDED */
