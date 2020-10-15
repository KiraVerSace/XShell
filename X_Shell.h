/*
 * @Description  :
 * @Version      : 0.1
 * @Company      : V-Think Development Team
 * @Author       : KiraVerSace@yeah.net
 * @Date         : 2020-10-04 04:00:18
 * @LastEditTime : 2020-10-10 03:52:09
 * @Attention	 : The code is refer to https://github.com/Nrusher/nr_micro_shell
 */
#ifndef __X_SHELL_H__
#define __X_SHELL_H__

#include <Arduino.h>

/*------------------------- x_micro_shell_conf.h ------------------------*/
#define X_SHELL_VERSION					("\r\nV1.0 2020-10-09 Author by KiraVerSace\r\n")
/* The user's name. */
#define X_SHELL_USER_NAME 				"VThink > "
/* Maximum user name length. */
#define X_SHELL_USER_NAME_MAX_LENGTH 	30
/* Enable or disable the echo */
#define X_SHELL_ECHO_ENABLE				1
/* Maximum count of command. 最多允许的命令个数*/
#define X_SHELL_MAX_CMD_COUNT		 	64
/* ANSI command line buffer size. 允许的命令行最大长度 */
#define X_ANSI_LINE_SIZE 				100
/* Maximum command name length. 命令行中命令名称最大长度*/
#define X_SHELL_CMD_NAME_MAX_LENGTH 	10
/* Command line buffer size. 允许命令的最大长度*/
#define X_SHELL_CMD_LINE_MAX_LENGTH 	X_ANSI_LINE_SIZE
/* The maximum number of parameters in the command. 命令参数的最多个数*/
#define X_SHELL_CMD_PARAS_MAX_NUM 		10
/* Command stores the most history commands (refers to X_SHELL_MAX_CMD_COUNT)*/
#define X_SHELL_MAX_CMD_HISTORY_NUM 	3
/* History command cache length */
#define X_SHELL_CMD_HISTORY_BUF_LENGTH	128
/*  [0: \n]/Unix   [1: \r]/MacOS   [2: \r\n]/Windows */
#define X_SHELL_END_OF_LINE 			1
/* Weather the terminal support all ANSI codes. When you use secureCRT please set to 0 */
#define X_SHLL_FULL_ANSI 				1
/* Show logo or not. */
#define X_SHELL_SHOW_LOGO 				1


/*---------- type definitions -----------*/
typedef struct
{
	char commandName[X_SHELL_CMD_NAME_MAX_LENGTH];
	void (*functionPoint)(char argc, char *argv);
} CommandT;
typedef struct
{
	uint16_t fp;
	uint16_t rp;

	uint16_t len;
	uint16_t index;

	uint16_t storeFront;
	uint16_t storeRear;
	uint16_t storeNum;

	char queue[X_SHELL_MAX_CMD_HISTORY_NUM  + 1];
	char buf[X_SHELL_CMD_HISTORY_BUF_LENGTH + 1];
} ShellHistoryQueueT;
typedef struct
{
	char  userName[X_SHELL_USER_NAME_MAX_LENGTH];
	const CommandT *command;
	ShellHistoryQueueT shellHistoryQueue;
} ShellT;

typedef void (*ShellFunctionT)(char, char *);

class X_Shell
{
public:
	X_Shell();
	~X_Shell();

	void addCommand(const char *name, ShellFunctionT functionPoint);
	void init(HardwareSerial &serial);
	void run(void);
private:
	HardwareSerial *_shellSerial;
	static uint16_t _commandCount;
	static CommandT _staticCommand[X_SHELL_MAX_CMD_COUNT];

	static void shellListCommand(char argc, char *argv);
	static void shellParseCommand(char argc, char *argv);
/*------------------------- x_micro_shell.h ------------------------*/
	ShellT _shell;

	char *shellStrtok(char *stringOrg, const char *demial);
	void shellIinit(ShellT *shell);
	ShellFunctionT shellCommandSerach(ShellT *shell, char *str);
    void shellParser(ShellT *shell, char *str);
    char *shellCommandComplete(ShellT *shell, char *str);
    void shellHistoryQueueInit(ShellHistoryQueueT *queueHistory);
    void shellHistoryQueueCommandAdd(ShellHistoryQueueT *queueHistory, char *str);
    uint16_t shellHistoryQueueCommandSerach(ShellHistoryQueueT *queueHistory, char *str);
    void shellHistoryQueueItemCopy(ShellHistoryQueueT *queueHistory, uint16_t i, char *strBuf);

	#if X_SHELL_END_OF_LINE == 1
	#define X_SHELL_END_CHAR '\r'
	#else
	#define X_SHELL_END_CHAR '\n'
	#endif

	#if X_SHELL_END_OF_LINE == 0
	#define X_SHELL_NEXT_LINE "\n"
	#endif

	#if X_SHELL_END_OF_LINE == 1
	#define X_SHELL_NEXT_LINE "\r\n"
	#endif

	#if X_SHELL_END_OF_LINE == 2
	#define X_SHELL_NEXT_LINE "\r\n"
	#endif
/*------------------------- ansi_def.h ------------------------*/
	#define X_ANSI_CTRL_MAX_LEN 	20
	#define X_ANSI_MAX_EX_DATA_NUM 	1

	enum
    {
        ANSI_ENABLE_SHOW,
        ANSI_DISABLE_SHOW
    };

	enum
	{
		ANSI_NO_CTRL_CHAR,
		ANSI_MAYBE_CTRL_CHAR,
		ANSI_WAIT_CTRL_CHAR_END,
	};

    typedef struct x_ansi_struct
    {
        int16_t p;
        int16_t counter;
        char currentLine[X_ANSI_LINE_SIZE];

        char combineBuf[X_ANSI_CTRL_MAX_LEN];
        uint8_t cmdNum;
        char combineState;
    } AnsiT;

	AnsiT _ansi;

	#define X_ANSI_SET_TEXT(cmd) 	((const char *)"\033["##cmd##"m") /** the form of set text font */
	/** set the color of background */
	#define X_ANSI_BBLACK 			"40"
	#define X_ANSI_BRED 			"41"
	#define X_ANSI_BGREEN 			"42"
	#define X_ANSI_BGRAY 			"43"
	#define X_ANSI_BBLUE 			"44"
	#define X_ANSI_BPURPLE 			"45"
	#define X_ANSI_BAQUAM 			"46"
	#define X_ANSI_BWHITE 			"47"

	/** set the color of character */
	#define X_ANSI_FBLACK 			"30"
	#define X_ANSI_FRED 			"31"
	#define X_ANSI_FGREEN 			"32"
	#define X_ANSI_FGRAY 			"33"
	#define X_ANSI_FBLUE 			"34"
	#define X_ANSI_FPURPLE 			"35"
	#define X_ANSI_FAQUAM 			"36"
	#define X_ANSI_FWHITE 			"37"

	/** special effect */
	#define X_ANSI_NORMAL 			"0"
	#define X_ANSI_BRIGHT 			"1"
	#define X_ANSI_UNDERLINE 		"4"
	#define X_ANSI_FLASH 			"5"
	#define X_ANSI_INVERSE 			"7"
	#define X_ANSI_INVISABLE 		"8"

	/** clear code */
	#define X_ANSI_CLEAR_RIGHT 		"\033[K"
	#define X_ANSI_CLEAR_LEFT 		"\033[1K"
	#define X_ANSI_CLEAR_WHOLE 		"\033[2K"

	#define X_ANSI_CLEAR_SCREEN 	"\033[2J"

	#define X_ANSI_HIDE_COURSER 	"\033[?25l"
	#define X_ANSI_SHOW_COURSER 	"\033[?25h"

	#define X_ANSI_SET_FONT(cmd) 			((const char *)"\033["#cmd"I")
	#define X_ANSI_CLR_R_NCHAR(cmd) 		((const char *)"\033["#cmd"X")
	#define X_ANSI_CLR_R_MV_L_NCHAR(cmd) 	((const char *)"\033["#cmd"P")

	/** move course code */
	#define X_ANSI_MV_L_N(n) 				((const char *)"\033["#n"D")
	#define X_ANSI_MV_R_N(n) 				((const char *)"\033["#n"C")

	#define X_ANSI_NORMAL 	"0"
	#define X_ANSI_SONG 	"1"
	#define X_ANSI_HEI 		"2"
	#define X_ANSI_KAI 		"3"
/*------------------------- ansi_port.h ------------------------*/

	void ansiCtrlCommonSlover(AnsiT *ansi);
	void ansiCommonCharSlover(AnsiT *ansi, char ch);

	void ansiNewline(AnsiT *ansi);
	void ansiBackspace(AnsiT *ansi);
	void ansiUp(AnsiT *ansi);
	void ansiDown(AnsiT *ansi);
	void ansiLeft(AnsiT *ansi);
	void ansiRight(AnsiT *ansi);
	void ansiTab(AnsiT *ansi);
	void ansiEnter(AnsiT *ansi);
	void ansiReserveKey(AnsiT *ansi);

/*------------------------- ansi.h ------------------------*/
	void ansiInit(AnsiT *ansi);
	int16_t ansiCharSearch(char ch, const char *buf);
	char ansiCharGet(char ch, AnsiT *ansi);
	void ansiCurrentLineClear(AnsiT *ansi);

	const char ANSI_IN_COMMAND[32] = {'m', 'I', 'A', 'B', 'C', 'D', 'X', 'K', 'M', 'P', 'J', '@', 'L', 'l', 'h', 'n', 'H', 's', 'u', '~','\0'};
	const char ANSI_IN_SPECIAL_SYMBOL[8] = {'\b', '\n', '\r', '\t', '\0'};
};

extern X_Shell xShell;

#endif
