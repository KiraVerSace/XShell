# XShell
An extensible command shell for Arduino. 

refer from the rt-thread's finsh> and nr_micro_shell!

## How to use it?
### 1. Include the library:
```cpp
#include <X_Shell.h>
```

### 2. Create the object and initialize it:
```cpp
void setup(void)
{
    Serial.beigin(115200);
    xShell.init(Serial);
}
```

### 3. Loop the run function:
```cpp
void loop(void)
{
    xShell.run();
}
```
## Add a test command
```cpp
 xShell.addCommand(const char *name, ShellFunctionT functionPoint);
```
## Some important definitions in .h file
```cpp
/* The user's name. */
#define X_SHELL_USER_NAME 				"VThink > "
/*  [0: \n]/Unix   [1: \r]/MacOS   [2: \r\n]/Windows */
#define X_SHELL_END_OF_LINE 			        1
/* Show logo or not. */
#define X_SHELL_SHOW_LOGO 				1
/* Open or close the ECHO */
#define X_SHELL_ECHO_ENABLE			 	1
```

