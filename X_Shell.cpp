/*
 * @Description  :
 * @Version      : 0.1
 * @Company      : V-Think Development Team
 * @Author       : KiraVerSace@yeah.net
 * @Date         : 2020-10-04 04:04:29
 * @LastEditTime : 2020-10-08 23:54:00
 */
#include "X_Shell.h"

X_Shell::X_Shell(HardwareSerial &serial)
{
	this->_shellSerial = &serial;

	strncpy(this->_shell.userName, X_SHELL_USER_NAME, sizeof(this->_shell.userName));
	this->_shell.command = x_cmd_start_add;
}

void X_Shell::init(void)
{
	this->ansiInit(&this->_ansi);
	this->shellIinit(&this->_shell);
}


void X_Shell::run(char getCharacter)
{
	if (this->ansiCharGet(getCharacter, &this->_ansi) == X_SHELL_END_CHAR)
	{
		this->shellParser(&_shell, _ansi.currentLine);
		this->ansiCurrentLineClear(&_ansi);
	}
}

char *X_Shell::shellStrtok(char *stringOrg, const char *demial)
{
	static uint8_t *last;
	uint8_t *str;
	const uint8_t *ctrl = (const uint8_t *)demial;
	uint8_t map[32];
	int16_t count;

	for (count = 0; count < 32; count++)
	{
		map[count] = 0;
	}
	do
	{
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);
	if (stringOrg)
	{
		str = (uint8_t *)stringOrg;
	}
	else
	{
		str = last;
	}
	while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
	{
		str++;
	}
	stringOrg = (char *)str;
	for (; *str; str++)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
		{
			*str++ = '\0';
			break;
		}
	}
	last = str;
	if (stringOrg == (char *)str)
	{
		return NULL;
	}
	else
	{
		return stringOrg;
	}
}

void X_Shell::shellIinit(ShellT *shell)
{
#if (X_SHELL_SHOW_LOGO == 1)
	this->_shellSerial->print("__  __ __  _            _  _ \r\n");
	this->_shellSerial->print("\\ \\/ // _\\| |__    ___ | || |\r\n");
	this->_shellSerial->print(" \\  / \\ \\ | '_ \\  / _ \\| || |\r\n");
	this->_shellSerial->print(" /  \\ _\\ \\| | | ||  __/| || |\r\n");
	this->_shellSerial->print("/_/\\_\\\\__/|_| |_| \\___||_||_|\r\n");
#endif
	this->_shellSerial->print(shell->userName);
	shellHistoryQueueInit(&shell->shellHistoryQueue);
	shellHistoryQueueCommandAdd(&shell->shellHistoryQueue, (char *)"ls Command");
	shell->shellHistoryQueue.index = 1;
}

ShellFunctionT X_Shell::shellCommandSerach(ShellT *shell, char *str)
{
	uint16_t i = 0;
	while (shell->command[i].functionPoint != NULL)
	{
		if (!strcmp(str, shell->command[i].commandName))
		{
			return shell->command[i].functionPoint;
		}

		i++;
	}

	return NULL;
}

void X_Shell::shellParser(ShellT *shell, char *str)
{
	uint8_t argc = 0;
	char argv[X_SHELL_CMD_LINE_MAX_LENGTH + X_SHELL_CMD_PARAS_MAX_NUM];
	char *token = str;
	ShellFunctionT fp;
	char index = X_SHELL_CMD_PARAS_MAX_NUM;

	if (shellHistoryQueueCommandSerach(&shell->shellHistoryQueue, str) == 0 && str[0] != '\0')
	{
		shellHistoryQueueCommandAdd(&shell->shellHistoryQueue, str);
	}

	if (strlen(str) > X_SHELL_CMD_LINE_MAX_LENGTH)
	{
		this->_shellSerial->print("This command is too long.\r\n");
		this->_shellSerial->print(shell->userName);
		return;
	}

	token = shellStrtok(token, " ");
	fp = shellCommandSerach(shell, str);

	if (fp == NULL)
	{
		if (isalpha(str[0]))
		{
			this->_shellSerial->printf("No command named: %s !\r\n", token);
		}
	}
	else
	{
		argv[argc] = index;
		strcpy(argv + index, str);
		index += strlen(str) + 1;
		argc++;

		token = shellStrtok(NULL, " ");
		while (token != NULL)
		{
			argv[argc] = index;
			strcpy(argv + index, token);
			index += strlen(token) + 1;
			argc++;
			token = shellStrtok(NULL, " ");
		}
	}

	if (fp != NULL)
	{
		fp(argc, argv);
	}

	this->_shellSerial->print(shell->userName);
}

char* X_Shell::shellCommandComplete(ShellT *shell, char *str)
{
	char *temp = NULL;
	uint8_t i;
	char *best_matched = NULL;
	uint8_t min_position = 255;

	for (i = 0; shell->command[i].commandName[0] != '\0'; i++)
	{
		temp = NULL;
		temp = strstr(shell->command[i].commandName, str);
		if (temp != NULL && ((unsigned long)temp - (unsigned long)(&shell->command[i]) < min_position))
		{
			min_position = (unsigned long)temp - (unsigned long)(&shell->command[i]);
			best_matched = (char *)&shell->command[i];
			if (min_position == 0)
			{
				break;
			}
		}
	}

	return best_matched;
}

void X_Shell::shellHistoryQueueInit(ShellHistoryQueueT *queueHistory)
{
	queueHistory->fp = 0;
	queueHistory->rp = 0;
	queueHistory->len = 0;

	queueHistory->storeFront = 0;
	queueHistory->storeRear = 0;
	queueHistory->storeNum = 0;
}

void X_Shell::shellHistoryQueueCommandAdd(ShellHistoryQueueT *queueHistory, char *str)
{
	uint16_t str_len;
	uint16_t i;

	str_len = strlen(str);

	if (str_len > X_SHELL_CMD_HISTORY_BUF_LENGTH)
	{
		return;
	}

	while (str_len > (X_SHELL_CMD_HISTORY_BUF_LENGTH - queueHistory->storeNum) || queueHistory->len == X_SHELL_MAX_CMD_HISTORY_NUM)
	{

		queueHistory->fp++;
		queueHistory->fp = (queueHistory->fp > X_SHELL_MAX_CMD_HISTORY_NUM) ? 0 : queueHistory->fp;
		queueHistory->len--;

		if (queueHistory->storeFront <= queueHistory->queue[queueHistory->fp])
		{
			queueHistory->storeNum -= queueHistory->queue[queueHistory->fp] - queueHistory->storeFront;
		}
		else
		{
			queueHistory->storeNum -= queueHistory->queue[queueHistory->fp] + X_SHELL_CMD_HISTORY_BUF_LENGTH - queueHistory->storeFront + 1;
		}

		queueHistory->storeFront = queueHistory->queue[queueHistory->fp];
	}

	queueHistory->queue[queueHistory->rp] = queueHistory->storeRear;
	queueHistory->rp++;
	queueHistory->rp = (queueHistory->rp > X_SHELL_MAX_CMD_HISTORY_NUM) ? 0 : queueHistory->rp;
	queueHistory->len++;

	for (i = 0; i < str_len; i++)
	{
		queueHistory->buf[queueHistory->storeRear] = str[i];
		queueHistory->storeRear++;
		queueHistory->storeRear = (queueHistory->storeRear > X_SHELL_CMD_HISTORY_BUF_LENGTH) ? 0 : queueHistory->storeRear;
		queueHistory->storeNum++;
	}
	queueHistory->queue[queueHistory->rp] = queueHistory->storeRear;
}

uint16_t X_Shell::shellHistoryQueueCommandSerach(ShellHistoryQueueT *queueHistory, char *str)
{
	uint16_t str_len;
	uint16_t i, j;
	uint16_t index_temp = queueHistory->fp;
	uint16_t start;
	uint16_t end;
	uint16_t cmd_len;
	uint16_t matched_id = 0;
	uint16_t buf_index;

	if (queueHistory->len == 0)
	{
		return matched_id;
	}
	else
	{
		str_len = strlen(str);
		for (i = 0; i < queueHistory->len; i++)
		{
			start = queueHistory->queue[index_temp];
			index_temp++;
			index_temp = (index_temp > X_SHELL_MAX_CMD_HISTORY_NUM) ? 0 : index_temp;
			end = queueHistory->queue[index_temp];

			if (start <= end)
			{
				cmd_len = end - start;
			}
			else
			{
				cmd_len = X_SHELL_CMD_HISTORY_BUF_LENGTH + 1 - start + end;
			}

			if (cmd_len == str_len)
			{
				matched_id = i + 1;
				buf_index = start;
				for (j = 0; j < str_len; j++)
				{
					if (queueHistory->buf[buf_index] != str[j])
					{
						matched_id = 0;
						break;
					}

					buf_index++;
					buf_index = (buf_index > X_SHELL_CMD_HISTORY_BUF_LENGTH) ? 0 : buf_index;
				}

				if (matched_id != 0)
				{
					return matched_id;
				}
			}
		}

		return 0;
	}
}

void X_Shell::shellHistoryQueueItemCopy(ShellHistoryQueueT *queueHistory, uint16_t i, char *strBuf)
{
	uint16_t index_temp;
	uint16_t start;
	uint16_t end;
	uint16_t j;

	if (i <= queueHistory->len)
	{
		index_temp = queueHistory->fp + i - 1;
		index_temp = (index_temp > X_SHELL_MAX_CMD_HISTORY_NUM) ? (index_temp - X_SHELL_MAX_CMD_HISTORY_NUM - 1) : index_temp;

		start = queueHistory->queue[index_temp];
		index_temp++;
		index_temp = (index_temp > X_SHELL_MAX_CMD_HISTORY_NUM) ? 0 : index_temp;
		end = queueHistory->queue[index_temp];

		if (start < end)
		{
			for (j = start; j < end; j++)
			{
				strBuf[j - start] = queueHistory->buf[j];
			}

			strBuf[j - start] = '\0';
		}
		else
		{
			for (j = start; j < X_SHELL_CMD_HISTORY_BUF_LENGTH + 1; j++)
			{
				strBuf[j - start] = queueHistory->buf[j];
			}

			for (j = 0; j < end; j++)
			{
				strBuf[j + X_SHELL_CMD_HISTORY_BUF_LENGTH + 1 - start] = queueHistory->buf[j];
			}

			strBuf[j + X_SHELL_CMD_HISTORY_BUF_LENGTH + 1 - start] = '\0';
		}
	}
}


int16_t X_Shell::ansiCharSearch(char ch, const char *buf)
{
    int16_t i = 0;
    for (i = 0; (buf[i] != ch) && (buf[i] != '\0'); i++)
        ;
    if (buf[i] != '\0')
    {
        return i;
    }
    else
    {
        return -1;
    }
}

void X_Shell::ansiCommonCharSlover(AnsiT *ansi,char ch)
{
	short i;

	if (ansi->counter < X_ANSI_LINE_SIZE - 2)
    {
        if (ansi->p < ansi->counter)
        {
            for (i = ansi->counter; i > ansi->p; i--)
            {
                ansi->currentLine[i] = ansi->currentLine[i - 1];
            }
        }

        ansi->p++;
        ansi->counter++;

        ansi->currentLine[ansi->p] = ch;

        ansi->currentLine[ansi->counter] = '\0';
		if(ansi->p+1 < ansi->counter)
		{
			this->_shellSerial->print("\033[1@");
		}

#if (X_SHELL_ECHO_ENABLE == 1)
		this->_shellSerial->print(ch);
#endif
    }
    else
    {
        ansi->counter = X_ANSI_LINE_SIZE - 3;
        if (ansi->p >= ansi->counter)
        {
            ansi->p = ansi->counter - 1;
        }
        ansi->currentLine[ansi->counter] = '\0';
    }


}

void X_Shell::ansiCtrlCommonSlover(AnsiT *ansi)
{
    uint16_t i;
    for (i = 0; i < ansi->cmdNum; i++)
    {
		this->_shellSerial->print(*(ansi->combineBuf + i));
    }
}

// line break '\r' processing
void X_Shell::ansiEnter(AnsiT *ansi)
{
#if X_SHELL_END_OF_LINE == 1
	ansi->p = -1;
    ansi->counter = 0;

    _shell.shellHistoryQueue.index = _shell.shellHistoryQueue.len;
    this->_shellSerial->print("\r\n");
#else
    this->_shellSerial->print("\r");
#endif
}

// line break '\n' processing
void X_Shell::ansiNewline(AnsiT *ansi)
{
	ansi->p = -1;
    ansi->counter = 0;

    _shell.shellHistoryQueue.index = _shell.shellHistoryQueue.len;

#if X_SHELL_END_OF_LINE != 1
    this->_shellSerial->print("\r");
    this->_shellSerial->print("\n");
#else
    this->_shellSerial->print("\n");
#endif
}

// Backspace '\b' processing
void X_Shell::ansiBackspace(AnsiT *ansi)
{
    short i;

    if (ansi->p >= 0)
    {
        for (i = ansi->p; i < ansi->counter; i++)
        {
            ansi->currentLine[i] = ansi->currentLine[i + 1];
        }

        ansi->p--;
        ansi->counter--;

        this->_shellSerial->print("\b");
#if X_SHLL_FULL_ANSI == 1
        this->_shellSerial->print(X_ANSI_CLR_R_MV_L_NCHAR(1));
#endif
    }
}

// up key processing
void X_Shell::ansiUp(AnsiT *ansi)
{
    if (_shell.shellHistoryQueue.index > 0)
    {
#if X_SHLL_FULL_ANSI == 1
        this->_shellSerial->printf("\033[%dD", ansi->p + 1);
        this->_shellSerial->print(X_ANSI_CLEAR_RIGHT);
#else
        this->_shellSerial->print("\r\n");
        this->_shellSerial->print(_shell.userName);
#endif

        shellHistoryQueueItemCopy(&_shell.shellHistoryQueue, _shell.shellHistoryQueue.index, ansi->currentLine);
        ansi->counter = strlen(ansi->currentLine);
        ansi->p = ansi->counter - 1;

        for (uint16_t i=0; i< ansi->counter; i++)
		{
			this->_shellSerial->print(ansi->currentLine[i]);
		}

		_shell.shellHistoryQueue.index--;
        _shell.shellHistoryQueue.index = (_shell.shellHistoryQueue.index == 0) ? _shell.shellHistoryQueue.len : _shell.shellHistoryQueue.index;
    }
}

// down key processing
void X_Shell::ansiDown(AnsiT *ansi)
{
    if (_shell.shellHistoryQueue.index > 0)
    {
#if X_SHLL_FULL_ANSI == 1
        this->_shellSerial->printf("\033[%dD", ansi->p + 1);
        this->_shellSerial->print(X_ANSI_CLEAR_RIGHT);
#else
        this->_shellSerial->print("\r\n");
        this->_shellSerial->print(_shell.userName);
#endif

        shellHistoryQueueItemCopy(&_shell.shellHistoryQueue, _shell.shellHistoryQueue.index, ansi->currentLine);
        ansi->counter = strlen(ansi->currentLine);
        ansi->p = ansi->counter - 1;

		for (uint16_t i=0; i< ansi->counter; i++)
		{
			this->_shellSerial->print(ansi->currentLine[i]);
		}

		_shell.shellHistoryQueue.index++;
        _shell.shellHistoryQueue.index = (_shell.shellHistoryQueue.index > _shell.shellHistoryQueue.len) ? 1 : _shell.shellHistoryQueue.index;
    }
}

// left key <- processing
void X_Shell::ansiLeft(AnsiT *ansi)
{
    if (ansi->p > -1)
    {
        ansi->p--;
#if X_SHLL_FULL_ANSI == 1
        this->_shellSerial->print("\033[1D");
#endif
    }
}

// right key <- processing
void X_Shell::ansiRight(AnsiT *ansi)
{
    if (ansi->p < (int16_t)(ansi->counter - 1))
    {
        ansi->p++;
#if X_SHLL_FULL_ANSI == 1
        this->_shellSerial->print("\033[1C");
#endif
    }
}

// tab key processing
void X_Shell::ansiTab(AnsiT *ansi)
{
    uint8_t i;
    char *cmd;
    cmd = shellCommandComplete(&_shell, ansi->currentLine);
    if (cmd != NULL)
    {

        if (ansi->counter == 0)
        {
            this->_shellSerial->print("\r\n");
            for (i = 0; _shell.command[i].functionPoint != NULL; i++)
            {
                this->_shellSerial->print(_shell.command[i].commandName);
                this->_shellSerial->print("\r\n");
            }

            this->_shellSerial->print(_shell.userName);
        }
        else
        {
#if X_SHLL_FULL_ANSI == 1
            this->_shellSerial->printf("\033[%dD", ansi->p + 1);
            this->_shellSerial->print(X_ANSI_CLEAR_RIGHT);
#else
            this->_shellSerial->print("\r\n");
            this->_shellSerial->print(_shell.userName);
#endif
            ansi->counter = strlen(cmd);
            ansi->p = ansi->counter - 1;
            strcpy(ansi->currentLine, cmd);

			for (uint16_t i=0; i< ansi->counter; i++)
			{
				this->_shellSerial->print(ansi->currentLine[i]);
			}
        }
    }
}

/*ansi delete*/
void X_Shell::ansiReserveKey(AnsiT *ansi)
{
	int16_t i;
	if(ansi->combineBuf[2] == '3')
	{
		for(i = ansi->p+1;i<ansi->counter;i++)
		{
			ansi->currentLine[i] = ansi->currentLine[i+1];
		}
		if((short)ansi->counter > ansi->p)
		{
			ansi->counter--;
#if X_SHLL_FULL_ANSI == 1
			this->_shellSerial->print("\033[1P");
#endif
		}

	}
}


void X_Shell::ansiInit(AnsiT *ansi)
{
    ansi->counter = 0;
    ansi->p = -1;

    ansi->currentLine[ansi->counter] = '\0';

    ansi->cmdNum = 0;
    ansi->combineState = ANSI_NO_CTRL_CHAR;
}

void X_Shell::ansiCurrentLineClear(AnsiT *ansi)
{
    ansi->counter = 0;
    ansi->p = -1;

    ansi->currentLine[ansi->counter] = '\0';
}

char X_Shell::ansiCharGet(char ch, AnsiT *ansi)
{
    int16_t cmd_id = -1;

    if (ansi->combineState == ANSI_NO_CTRL_CHAR)
    {
        cmd_id = ansiCharSearch(ch,   ANSI_IN_SPECIAL_SYMBOL);
        if (cmd_id >= 0)
        {
			switch (cmd_id)
			{
			case 0:
			{
				this->ansiBackspace(ansi);
			}
			break;

			case 1:
			{
				this->ansiNewline(ansi);
			}
			break;

			case 2:
			{
				this->ansiEnter(ansi);
			}
			break;

			case 3:
			{
				this->ansiTab(ansi);
			}
			break;

			default:
				break;
			}
        }
        else if (ch == '\033')
        {
            ansi->combineState = ANSI_WAIT_CTRL_CHAR_END;
            ansi->combineBuf[ansi->cmdNum] = ch;
			ansi->cmdNum++;
        }
        else
        {
			ansiCommonCharSlover(ansi,ch);
        }
    }
    else if (ansi->combineState == ANSI_WAIT_CTRL_CHAR_END)
    {
        ansi->combineBuf[ansi->cmdNum] = ch;

        if (('a' <= ch && 'z' >= ch) || ('A' <= ch && 'Z' >= ch) || ch== '~')
        {
            cmd_id = ansiCharSearch(ch, ANSI_IN_COMMAND);
			switch (cmd_id)
			{
				case 0:
				case 1:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				{
					this->ansiCtrlCommonSlover(ansi);
				}
				break;

				case 2:
				{
					this->ansiUp(ansi);
				}
				break;

				case 3:
				{
					this->ansiDown(ansi);
				}
				break;

				case 4:
				{
					this->ansiRight(ansi);
				}
				break;

				case 5:
				{
					this->ansiLeft(ansi);
				}
				break;

				case 19:
				{
					this->ansiReserveKey(ansi);
				}
				break;
			default:
				break;
			}

            ansi->cmdNum = 0;
            ansi->combineState = ANSI_NO_CTRL_CHAR;
        }
        else if (ansi->cmdNum > 18)
        {
            ansi->cmdNum = 0;
            ansi->combineState = ANSI_NO_CTRL_CHAR;
        }
        else
        {
            ansi->cmdNum++;
        }
    }
    else
    {
        ansi->combineState = ANSI_NO_CTRL_CHAR;
    }

    return ch;
}


